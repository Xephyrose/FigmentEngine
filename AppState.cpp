#include "AppState.h"

#include <tiny_gltf.h>
#include <filesystem>
#include <SDL3_image/SDL_image.h>

#include "MaterialUnlitTextured.h"
#include "Mesh.h"
#include "Vertex.h"
#include "SDL3/SDL_log.h"

bool AppState::CreatePipeline(const std::string& name, const std::string& vertShader, const std::string& fragShader, const std::string& rasterizerState, const std::string &blendState) {
    SDL_GPUShader* vertexShader = GetShader(vertShader + ".vert");
    if (!vertexShader) {
        SDL_Log("Couldn't create vertex shader: %s", vertShader.c_str());
        return false;
    }

    SDL_GPUShader* fragmentShader = GetShader(fragShader + ".frag");
    if (!fragmentShader) {
        SDL_Log("Couldn't create fragment shader: %s", fragShader.c_str());
        return false;
    }

    constexpr std::array vertexBufferDescriptions{
        SDL_GPUVertexBufferDescription{
            .slot = 0,
            .pitch = sizeof(Vertex),
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
        },
    };

    constexpr std::array vertexAttributes{
        SDL_GPUVertexAttribute{
            .location = 0,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset = offsetof(Vertex, position),
        },
        SDL_GPUVertexAttribute{
            .location = 1,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = offsetof(Vertex, uv),
        }
    };

    const std::array colorTargetDescriptions{
        SDL_GPUColorTargetDescription{
            .format = SDL_GetGPUSwapchainTextureFormat(device, window),
            .blend_state = GetBlendState(blendState),
        }
    };

    const bool depth_write = blendState != "Alpha";
    const auto pipelineCreateInfo = SDL_GPUGraphicsPipelineCreateInfo{
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .vertex_input_state = SDL_GPUVertexInputState{
            .vertex_buffer_descriptions = vertexBufferDescriptions.data(),
            .num_vertex_buffers = vertexBufferDescriptions.size(),
            .vertex_attributes = vertexAttributes.data(),
            .num_vertex_attributes = vertexAttributes.size(),
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = GetRasterizerState(rasterizerState),
        .depth_stencil_state = SDL_GPUDepthStencilState{
            .compare_op = SDL_GPU_COMPAREOP_LESS,
            .enable_depth_test = true,
            .enable_depth_write = depth_write,
        },
        .target_info = SDL_GPUGraphicsPipelineTargetInfo{
            .color_target_descriptions = colorTargetDescriptions.data(),
            .num_color_targets = colorTargetDescriptions.size(),
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
            .has_depth_stencil_target = true
        },
    };

    pipelines.insert_or_assign(name, SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo));
    if (!pipelines[name]) {
        SDL_Log("Couldn't create graphics pipeline: %s", SDL_GetError());
        return false;
    }

    SDL_ReleaseGPUShader(device, vertexShader);
    SDL_ReleaseGPUShader(device, fragmentShader);
    return true;
}

static std::vector<float> ReadAttributeData(const tinygltf::Model& model, const int accessorIdx) {
    std::vector<float> result;

    if (accessorIdx < 0) return result;

    const tinygltf::Accessor& accessor = model.accessors[accessorIdx];
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

    size_t vertexCount = accessor.count;
    int numComponents = tinygltf::GetNumComponentsInType(accessor.type);

    result.resize(vertexCount * numComponents);

    // Handle different component types
    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
        const auto* floatData = reinterpret_cast<const float*>(dataPtr);
        for (size_t i = 0; i < vertexCount * numComponents; i++) {
            result[i] = floatData[i];
        }
    }
    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        const auto* shortData = reinterpret_cast<const uint16_t*>(dataPtr);
        for (size_t i = 0; i < vertexCount * numComponents; i++) {
            result[i] = static_cast<float>(shortData[i]);
        }
    }
    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
        const auto* byteData = reinterpret_cast<const uint8_t*>(dataPtr);
        for (size_t i = 0; i < vertexCount * numComponents; i++) {
            result[i] = static_cast<float>(byteData[i]) / 255.0f;
        }
    }

    return result;
}

bool AppState::LoadMesh(const std::string& path) {
    std::string fullPath = (std::filesystem::path(SDL_GetBasePath()) / "assets" / "meshes" / path).string();
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool success = loader.LoadBinaryFromFile(&model, &err, &warn, fullPath);

    if (!warn.empty()) {
        SDL_Log("GLTF Warning: %s", warn.c_str());
    }
    if (!err.empty()) {
        SDL_Log("GLTF Error: %s", err.c_str());
    }
    if (!success) {
        SDL_Log("Failed to load GLB file: %s", fullPath.c_str());
        return {};
    }

    Mesh result;

    // NEW: Build node-to-mesh mapping
    std::vector<std::pair<int, std::string>> meshToNodeName; // meshIndex -> (nodeIndex, nodeName)

    for (const auto & node : model.nodes) {
        if (node.mesh >= 0) {
            meshToNodeName.emplace_back(node.mesh, node.name);
            // SDL_Log("Node[%zu]: name='%s' -> mesh=%d", nodeIdx, node.name.c_str(), node.mesh);
        }
    }

    // Process all meshes
    for (int meshIdx = 0; meshIdx < static_cast<int>(model.meshes.size()); meshIdx++) {
        const auto& gltfMesh = model.meshes[meshIdx];

        // Find which node(s) use this mesh
        std::string objectName = "Unnamed";
        for (const auto& mapping : meshToNodeName) {
            if (mapping.first == meshIdx) {
                objectName = mapping.second;
                break;
            }
        }

        // SDL_Log("Processing mesh %d: name='%s' (object: '%s')", meshIdx, gltfMesh.name.c_str(), objectName.c_str());

        for (const auto& primitive : gltfMesh.primitives) {
            Submesh submesh;
            submesh.name = objectName;
            submesh.meshName = gltfMesh.name;
            submesh.startVertex = result.vertices.size();
            submesh.startIndex = result.indices.size();

            size_t startVertex = result.vertices.size();

            // Read POSITION attribute
            auto posIt = primitive.attributes.find("POSITION");
            if (posIt != primitive.attributes.end()) {
                std::vector<float> positions = ReadAttributeData(model, posIt->second);
                size_t vertexCount = positions.size() / 3;
                result.vertices.resize(startVertex + vertexCount);

                for (size_t i = 0; i < vertexCount; i++) {
                    result.vertices[startVertex + i].position = glm::vec3(
                        positions[i * 3],
                        positions[i * 3 + 1],
                        positions[i * 3 + 2]
                    );
                    result.vertices[startVertex + i].uv = glm::vec2(0.0f, 0.0f);
                }
                submesh.vertexCount = vertexCount;
            }

            // Read NORMAL attribute (add this for better lighting)
            auto normIt = primitive.attributes.find("NORMAL");
            if (normIt != primitive.attributes.end()) {
                std::vector<float> normals = ReadAttributeData(model, normIt->second);
                size_t vertexCount = normals.size() / 3;
                for (size_t i = 0; i < vertexCount && i < submesh.vertexCount; i++) {
                    // You'll need to add a 'normal' member to your Vertex struct
                    // result.vertices[startVertex + i].normal = glm::vec3(
                    //     normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]
                    // );
                }
            }

            // Read TEXCOORD_0 attribute (UVs)
            auto uvIt = primitive.attributes.find("TEXCOORD_0");
            if (uvIt != primitive.attributes.end()) {
                std::vector<float> uvs = ReadAttributeData(model, uvIt->second);
                size_t vertexCount = uvs.size() / 2;
                for (size_t i = 0; i < vertexCount && i < submesh.vertexCount; i++) {
                    result.vertices[startVertex + i].uv = glm::vec2(uvs[i * 2], uvs[i * 2 + 1]);
                }
            }

            // Read indices
            if (primitive.indices >= 0) {
                const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + indexAccessor.byteOffset;
                size_t indexCount = indexAccessor.count;

                result.indices.reserve(result.indices.size() + indexCount);

                if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const auto* indices16 = reinterpret_cast<const uint16_t*>(dataPtr);
                    for (size_t i = 0; i < indexCount; i++) {
                        result.indices.push_back(static_cast<uint16_t>(indices16[i] + startVertex));
                    }
                }
                else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const auto* indices32 = reinterpret_cast<const uint32_t*>(dataPtr);
                    for (size_t i = 0; i < indexCount; i++) {
                        if (indices32[i] + startVertex > 65535) {
                            throw std::runtime_error("Model has more than 65535 vertices. Consider using uint32_t for indices.");
                        }
                        result.indices.push_back(static_cast<uint16_t>(indices32[i] + startVertex));
                    }
                }
                else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    const auto* indices8 = reinterpret_cast<const uint8_t*>(dataPtr);
                    for (size_t i = 0; i < indexCount; i++) {
                        result.indices.push_back(static_cast<uint16_t>(indices8[i] + startVertex));
                    }
                }

                submesh.indexCount = indexCount;
            }

            submesh.material = model.materials[primitive.material].name;

            result.submeshes.push_back(submesh);
            // SDL_Log("  Added submesh '%s' - vertices: %zu, indices: %zu", submesh.name.c_str(), submesh.vertexCount, submesh.indexCount);
        }
    }

    // SDL_Log("=== Mesh Loading Complete ===");
    // SDL_Log("Total vertices: %zu", result.vertices.size());
    // SDL_Log("Total indices: %zu", result.indices.size());
    // SDL_Log("Submeshes found: %zu", result.submeshes.size());

    // for (const auto& submesh : result.submeshes) {
    //     SDL_Log("  - Object: '%s' (Mesh: '%s') vtx[%zu-%zu] idx[%zu-%zu]",
    //             submesh.name.c_str(),
    //             submesh.meshName.c_str(),
    //             submesh.startVertex,
    //             submesh.startVertex + submesh.vertexCount - 1,
    //             submesh.startIndex,
    //             submesh.startIndex + submesh.indexCount - 1);
    // }

    meshes.insert_or_assign(path, result);
    meshes.at(path).UploadToGPU(*this);

    return true;
}

bool AppState::LoadShader(const std::string& path) {
    const std::string fullPath = (std::filesystem::path(SDL_GetBasePath()) / "assets" / "shaders" / path).string();
    SDL_GPUShaderStage stage;
    if (fullPath.contains(".vert"))
    {
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    }
    else if (fullPath.contains(".frag"))
    {
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    }
    else
    {
        SDL_Log("Couldn't deduce shader stage from file name: %s", fullPath.c_str());
        return false;
    }

    // Starts as invalid so we don't assume
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    // Different shaer formats have different entrypoint names
    const char* entrypoint;

    std::string extension;
    SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
    if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV)
    {
        extension = ".spv";
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entrypoint = "main";
    }
    else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL)
    {
        extension = ".msl";
        format = SDL_GPU_SHADERFORMAT_MSL;
        entrypoint = "main0";
    }
    else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL)
    {
        extension = ".dxil";
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entrypoint = "main";
    }
    else
    {
        SDL_Log("Couldn't find a supported shader format for backend %s!", SDL_GetGPUDeviceDriver(device));
        return false;
    }

    // Store the size of the data we're loading, to be reused later
    size_t fileSize;
    void* code = SDL_LoadFile((fullPath + extension).c_str(), &fileSize);
    if (code == nullptr)
    {
        SDL_Log("Couldn't load shader file from disk!\n\t%s", SDL_GetError());
        return false;
    }

    const auto shaderInfo = SDL_GPUShaderCreateInfo{
        .code_size = fileSize,
        .code = static_cast<Uint8*>(code),
        .entrypoint = entrypoint,
        .format = format,
        .stage = stage,

        .num_samplers =
            stage == SDL_GPU_SHADERSTAGE_FRAGMENT ? 1u : 0u,

        .num_storage_textures = 0u,
        .num_storage_buffers = 0u,
        .num_uniform_buffers = 1u,
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
    if (shader == nullptr)
    {
        SDL_Log("Couldn't create shader from file %s: %s", fullPath.c_str(), SDL_GetError());
        SDL_free(code);
        return false;
    }

    shaders.insert_or_assign(path, shader);

    return true;
}

bool AppState::LoadTexture(const std::string& path) {
    const std::string fullPath = (std::filesystem::path(SDL_GetBasePath()) / "assets" / "textures" / path).string();
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
    if (!uploadCmdBuf) {
        SDL_Log("Couldn't acquire command buffer: %s", SDL_GetError());
        return false;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
    if (!copyPass) {
        SDL_Log("Couldn't begin copy pass: %s", SDL_GetError());
        SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
        return false;
    }

    // Load image
    SDL_GPUTexture* texture = IMG_LoadGPUTexture(device, copyPass, fullPath.c_str(), nullptr, nullptr);

    // End the copy pass
    SDL_EndGPUCopyPass(copyPass);

    if (!texture) {
        SDL_Log("Couldn't load texture: %s", path.c_str());
        SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
        return false;
    }

    textures.insert_or_assign(path, texture);

    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(uploadCmdBuf);
    SDL_WaitForGPUFences(device, true, &fence, 1);
    SDL_ReleaseGPUFence(device, fence);

    return true;
}

Mesh* AppState::GetMesh(const std::string& path) {
    if (!meshes.contains(path)) {
        if (!LoadMesh(path)) {
            SDL_Log("Couldn't load shader %s: %s", path.c_str(), SDL_GetError());
            return nullptr;
        }
    }
    return &meshes.at(path);
}

Material* AppState::GetMaterial(const std::string& key) const {
    if (!materials.contains(key)) {
        return nullptr;
    }
    return materials.at(key);
}

SDL_GPUShader* AppState::GetShader(const std::string& path) {
    if (!shaders.contains(path)) {
        if (!LoadShader(path)) {
            SDL_Log("Couldn't load shader %s: %s", path.c_str(), SDL_GetError());
            return nullptr;
        }
    }
    return shaders.at(path);
}

SDL_GPUSampler* AppState::GetSampler(const std::string& key) const {
    return samplers.at(key);
}

SDL_GPUTexture* AppState::GetTexture(const std::string &path) {
    if (!textures.contains(path)) {
        if (!LoadTexture(path)) {
            SDL_Log("Couldn't load shader %s: %s", path.c_str(), SDL_GetError());
            return textures.at("missing.png");
        }
    }
    return textures.at(path);
}

SDL_GPUGraphicsPipeline* AppState::GetPipeline(const std::string& key) const {
    return pipelines.at(key);
}

SDL_GPURasterizerState AppState::GetRasterizerState(const std::string &key) const {
    return rasterizerStates.at(key);
}

SDL_GPUColorTargetBlendState AppState::GetBlendState(const std::string &key) const {
    return blendStates.at(key);
}

void AppState::CreateDefaultMeshes() {
    LoadMesh("zulu.glb");
    LoadMesh("crate_medium.glb");
}

void AppState::CreateDefaultMaterials() {
    SDL_Log("Creating default materials...");
    auto* missing = new MaterialUnlitTextured(this, "missing", "UnlitTextured");
    missing->setTextureAlbedo(this, "missing.png");
    missing->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* line = new MaterialUnlitTextured(this, "line", "Line");
    line->setTextureAlbedo(this, "missing.png");
    line->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* concrete_bricks = new MaterialUnlitTextured(this, "concrete_bricks", "UnlitTextured");
    concrete_bricks->setTextureAlbedo(this, "brick_concrete_albedo.png");
    concrete_bricks->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* concrete_bricks_with_specks = new MaterialUnlitTextured(this, "concrete_bricks_with_specks", "UnlitTextured");
    concrete_bricks_with_specks->setTextureAlbedo(this, "brick_concrete_specks_albedo.png");
    concrete_bricks_with_specks->setSamplerAlbedo(this, "anisotropic_repeat");
    new MaterialUnlitTextured(this, "plaster", "UnlitTextured");
    auto* reinforced_glass = new MaterialUnlitTextured(this, "reinforced_glass", "UnlitTexturedAlpha");
    reinforced_glass->setTextureAlbedo(this, "reinforced_glass_albedo.png");
    reinforced_glass->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* fence = new MaterialUnlitTextured(this, "fence", "UnlitTexturedAlpha");
    fence->setTextureAlbedo(this, "fence_albedo.png");
    fence->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* asphalt = new MaterialUnlitTextured(this, "asphalt", "UnlitTextured");
    asphalt->setTextureAlbedo(this, "asphalt_albedo.png");
    asphalt->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* asphalt_2 = new MaterialUnlitTextured(this, "asphalt_2", "UnlitTextured");
    asphalt_2->setTextureAlbedo(this, "asphalt_2_albedo.png");
    asphalt_2->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* concrete = new MaterialUnlitTextured(this, "concrete", "UnlitTextured");
    concrete->setTextureAlbedo(this, "concrete_albedo.png");
    concrete->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* concrete_with_specks = new MaterialUnlitTextured(this, "concrete_with_specks", "UnlitTextured");
    concrete_with_specks->setTextureAlbedo(this, "concrete_specks_albedo.png");
    concrete_with_specks->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* hardwood_dark = new MaterialUnlitTextured(this, "hardwood_dark", "UnlitTextured");
    hardwood_dark->setTextureAlbedo(this, "hardwood_dark_albedo.png");
    hardwood_dark->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* hardwood_light = new MaterialUnlitTextured(this, "hardwood_light", "UnlitTextured");
    hardwood_light->setTextureAlbedo(this, "hardwood_light_albedo.png");
    hardwood_light->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* pine_end = new MaterialUnlitTextured(this, "pine_end", "UnlitTextured");
    pine_end->setTextureAlbedo(this, "pine_end_albedo.png");
    pine_end->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* pine_wood_dark = new MaterialUnlitTextured(this, "pine_wood_dark", "UnlitTextured");
    pine_wood_dark->setTextureAlbedo(this, "pine_wood_dark_albedo.png");
    pine_wood_dark->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* pine_wood_light = new MaterialUnlitTextured(this, "pine_wood_light", "UnlitTextured");
    pine_wood_light->setTextureAlbedo(this, "pine_wood_light_albedo.png");
    pine_wood_light->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* roof_tile = new MaterialUnlitTextured(this, "roof_tile", "UnlitTextured");
    roof_tile->setTextureAlbedo(this, "roof_tile_albedo.png");
    roof_tile->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* wood_plank = new MaterialUnlitTextured(this, "wood_plank", "UnlitTextured");
    wood_plank->setTextureAlbedo(this, "wood_plank_albedo.png");
    wood_plank->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* grid_grey = new MaterialUnlitTextured(this, "grid_grey", "UnlitTextured");
    grid_grey->setTextureAlbedo(this, "grid_grey_albedo.png");
    grid_grey->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* grid_orange = new MaterialUnlitTextured(this, "grid_orange", "UnlitTextured");
    grid_orange->setTextureAlbedo(this, "grid_orange_albedo.png");
    grid_orange->setSamplerAlbedo(this, "anisotropic_repeat");
    auto* paint_red = new MaterialUnlitTextured(this, "paint_red", "UnlitTextured");
    paint_red->setColorAlbedo(glm::vec4(0.5f, 0.25f, 0.25f, 1.0f));
    auto* paint_beige = new MaterialUnlitTextured(this, "paint_beige", "UnlitTextured");
    paint_beige->setColorAlbedo(glm::vec4(0.75f, 0.55f, 0.45f, 1.0f));
    auto* clip = new MaterialUnlitTextured(this, "clip", "UnlitTexturedAlpha");
    clip->setColorAlbedo(glm::vec4(1.0f, 0.0f, 0.0f, 0.5f));
    auto* glass = new MaterialUnlitTextured(this, "glass", "UnlitTexturedAlpha");
    glass->setColorAlbedo(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    auto* metal_silver = new MaterialUnlitTextured(this, "metal_silver", "UnlitTextured");
    metal_silver->setColorAlbedo(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    auto* blend_brick_concrete = new MaterialUnlitTextured(this, "blend_brick_concrete", "UnlitTextured");
    blend_brick_concrete->setColorAlbedo(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
}

void AppState::CreateDefaultSamplers() {
    SDL_Log("Creating default samplers...");
    // Linear sampler (for most 3D textures, smooth scaling)
    constexpr SDL_GPUSamplerCreateInfo linearSamplerInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .mip_lod_bias = 0.0f,
        .min_lod = 0.0f,
        .max_lod = FLT_MAX,
    };
    if (SDL_GPUSampler* linearSampler = SDL_CreateGPUSampler(device, &linearSamplerInfo)) {
        samplers["linear_repeat"] = linearSampler;
    } else {
        SDL_Log("Couldn't create linear sampler: %s", SDL_GetError());
    }

    // Linear sampler with clamp to edge (for UI, decals, etc.)
    constexpr SDL_GPUSamplerCreateInfo linearClampInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .mip_lod_bias = 0.0f,
        .min_lod = 0.0f,
        .max_lod = FLT_MAX,
    };
    if (SDL_GPUSampler* linearClampSampler = SDL_CreateGPUSampler(device, &linearClampInfo)) {
        samplers["linear_clamp"] = linearClampSampler;
    } else {
        SDL_Log("Couldn't create linear clamp sampler: %s", SDL_GetError());
    }

    // Point sampler (for pixel art, sharp pixelated look)
    constexpr SDL_GPUSamplerCreateInfo pointSamplerInfo = {
        .min_filter = SDL_GPU_FILTER_NEAREST,
        .mag_filter = SDL_GPU_FILTER_NEAREST,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .mip_lod_bias = 0.0f,
        .min_lod = 0.0f,
        .max_lod = FLT_MAX,
    };
    if (SDL_GPUSampler* pointSampler = SDL_CreateGPUSampler(device, &pointSamplerInfo)) {
        samplers["point_clamp"] = pointSampler;
    } else {
        SDL_Log("Couldn't create point sampler: %s", SDL_GetError());
    }

    // Point sampler with repeat (for tiled pixel art)
    constexpr SDL_GPUSamplerCreateInfo pointRepeatInfo = {
        .min_filter = SDL_GPU_FILTER_NEAREST,
        .mag_filter = SDL_GPU_FILTER_NEAREST,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .mip_lod_bias = 0.0f,
        .min_lod = 0.0f,
        .max_lod = FLT_MAX,
    };
    if (SDL_GPUSampler* pointRepeatSampler = SDL_CreateGPUSampler(device, &pointRepeatInfo)) {
        samplers["point_repeat"] = pointRepeatSampler;
    } else {
        SDL_Log("Couldn't create point repeat sampler: %s", SDL_GetError());
    }

    // Anisotropic sampler (for textures viewed at extreme angles, like ground)
    constexpr SDL_GPUSamplerCreateInfo anisotropicInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .mip_lod_bias = 0.0f,
        .max_anisotropy = 16.0f,
        .min_lod = 0.0f,
        .max_lod = FLT_MAX,
        .enable_anisotropy = true,
    };
    if (SDL_GPUSampler* anisotropicSampler = SDL_CreateGPUSampler(device, &anisotropicInfo)) {
        samplers["anisotropic_repeat"] = anisotropicSampler;
    } else {
        SDL_Log("Couldn't create anisotropic sampler: %s", SDL_GetError());
    }
}

void AppState::CreateDefaultTextures() {
    SDL_Log("Creating default textures...");
    LoadTexture("missing.png");
}

void AppState::CreateDefaultPipelines() {
    SDL_Log("Creating default pipelines...");
    CreatePipeline("Line", "UnlitTextured", "UnlitTextured", "Line", "Default");
    CreatePipeline("UnlitTextured", "UnlitTextured", "UnlitTextured", "Fill", "Default");
    CreatePipeline("UnlitTexturedAlpha", "UnlitTextured", "UnlitTextured", "FillNoBack", "Alpha");
    CreatePipeline("UnlitUVs", "UnlitTextured", "UnlitUVs", "Fill", "Default");
}

void AppState::CreateDefaultRasterizerStates() {
    SDL_Log("Creating default rasterizer states...");
    SDL_GPURasterizerState fill{};
    fill.fill_mode = SDL_GPU_FILLMODE_FILL;
    fill.cull_mode = SDL_GPU_CULLMODE_BACK;
    fill.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    rasterizerStates.insert_or_assign("Fill", fill);
    SDL_GPURasterizerState fillNoBack{};
    fillNoBack.fill_mode = SDL_GPU_FILLMODE_FILL;
    fillNoBack.cull_mode = SDL_GPU_CULLMODE_NONE;
    fillNoBack.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    rasterizerStates.insert_or_assign("FillNoBack", fill);
    auto line = SDL_GPURasterizerState {};
    line.fill_mode = SDL_GPU_FILLMODE_LINE;
    line.cull_mode = SDL_GPU_CULLMODE_BACK;
    line.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    rasterizerStates.insert_or_assign("Line", line);
}

void AppState::CreateDefaultBlendStates() {
    SDL_Log("Creating default blend states...");
    SDL_GPUColorTargetBlendState alphaBlendState = {};
    alphaBlendState.enable_blend = true;
    alphaBlendState.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    alphaBlendState.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    alphaBlendState.color_blend_op = SDL_GPU_BLENDOP_ADD;
    alphaBlendState.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    alphaBlendState.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    alphaBlendState.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    alphaBlendState.color_write_mask = SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G | SDL_GPU_COLORCOMPONENT_B | SDL_GPU_COLORCOMPONENT_A;
    blendStates.insert_or_assign("Alpha", alphaBlendState);
    SDL_GPUColorTargetBlendState defaultBlendState = {};
    blendStates.insert_or_assign("Default", defaultBlendState);
}