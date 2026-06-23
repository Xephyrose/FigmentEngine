#include "AppState.h"

#include <thirdparty/tiny_gltf.h>
#include <filesystem>
#include <SDL3_image/SDL_image.h>

#include "MaterialLit.h"
#include "MaterialLitTextured.h"
#include "MaterialUnlitTextured.h"
#include "Mesh.h"
#include "PointLight3DGPU.h"
#include "Vertex.h"
#include "SDL3/SDL_log.h"
#include "SDL3_shadercross/SDL_shadercross.h"

bool AppState::CreatePipeline(const std::string& name, const std::string& vertShader, const std::string& fragShader, const std::string& rasterizerState, const std::string &blendState, const
                              bool &depth_test, const bool &depth_write) {
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
        },
        SDL_GPUVertexAttribute{
            .location = 2,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset = offsetof(Vertex, normal),
        }
    };

    const std::array colorTargetDescriptions{
        SDL_GPUColorTargetDescription{
            .format = SDL_GetGPUSwapchainTextureFormat(device, window),
            .blend_state = GetBlendState(blendState),
        }
    };

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
        .multisample_state = GetMultisampleState("Multisample"),
        .depth_stencil_state = SDL_GPUDepthStencilState{
            .compare_op = SDL_GPU_COMPAREOP_LESS,
            .enable_depth_test = depth_test,
            .enable_depth_write = depth_write,
        },
        .target_info = SDL_GPUGraphicsPipelineTargetInfo{
            .color_target_descriptions = colorTargetDescriptions.data(),
            .num_color_targets = colorTargetDescriptions.size(),
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
            .has_depth_stencil_target = true,
        },
    };

    pipelines.insert_or_assign(name, SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo));
    if (!pipelines[name]) {
        SDL_Log("Couldn't create graphics pipeline: %s", SDL_GetError());
        return false;
    }

    return true;
}

static std::vector<float> ReadAttributeData(const tinygltf::Model& model, const int accessorIdx) {
    std::vector<float> result;

    if (accessorIdx < 0) return result;

    const tinygltf::Accessor& accessor = model.accessors[accessorIdx];
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

    const size_t vertexCount = accessor.count;
    const int numComponents = tinygltf::GetNumComponentsInType(accessor.type);

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

    std::vector<std::pair<int, std::string>> meshToNodeName; // meshIndex -> (nodeIndex, nodeName)

    for (const auto & node : model.nodes) {
        if (node.mesh >= 0) {
            meshToNodeName.emplace_back(node.mesh, node.name);
            // SDL_Log("Node[%zu]: name='%s' -> editorMesh=%d", nodeIdx, node.name.c_str(), node.editorMesh);
        }
    }

    // Process all meshes
    for (int meshIdx = 0; meshIdx < static_cast<int>(model.meshes.size()); meshIdx++) {
        const auto& gltfMesh = model.meshes[meshIdx];

        std::string objectName = "Unnamed";
        for (const auto& mapping : meshToNodeName) {
            if (mapping.first == meshIdx) {
                objectName = mapping.second;
                break;
            }
        }

        for (const auto& primitive : gltfMesh.primitives) {
            Submesh submesh;
            submesh.name = objectName;
            submesh.meshName = gltfMesh.name;
            submesh.startVertex = result.vertices.size();
            submesh.startIndex = result.indices.size();

            size_t startVertex = result.vertices.size();

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

            auto normIt = primitive.attributes.find("NORMAL");
            if (normIt != primitive.attributes.end()) {
                std::vector<float> normals = ReadAttributeData(model, normIt->second);
                size_t vertexCount = normals.size() / 3;
                for (size_t i = 0; i < vertexCount && i < submesh.vertexCount; i++) {
                    result.vertices[startVertex + i].normal = glm::vec3(
                        normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]
                    );
                }
            }

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
    const SDL_ShaderCross_GraphicsShaderMetadata* shader_meta = SDL_ShaderCross_ReflectGraphicsSPIRV(static_cast<Uint8*>(code), fileSize, 0);
    SDL_Log("Creating shader %s, num_samplers is %u, num_storage_textures is %u, num_storage_buffers is %u, num_uniform_buffers is %u", path.c_str(), shader_meta->resource_info.num_samplers, shader_meta->resource_info.num_storage_textures, shader_meta->resource_info.num_storage_buffers, shader_meta->resource_info.num_uniform_buffers);
    const auto shaderInfo = SDL_GPUShaderCreateInfo{
        .code_size = fileSize,
        .code = static_cast<Uint8*>(code),
        .entrypoint = entrypoint,
        .format = format,
        .stage = stage,
        .num_samplers = shader_meta->resource_info.num_samplers,
        .num_storage_textures = shader_meta->resource_info.num_storage_textures,
        .num_storage_buffers = shader_meta->resource_info.num_storage_buffers,
        .num_uniform_buffers = shader_meta->resource_info.num_uniform_buffers,
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
            SDL_Log("Couldn't load mesh %s: %s", path.c_str(), SDL_GetError());
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
    if (path == "none") return textures.at("missing.png");
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

SDL_GPUMultisampleState AppState::GetMultisampleState(const std::string& key) const
{
    return multisampleStates.at(key);
}

SDL_GPUColorTargetBlendState AppState::GetBlendState(const std::string &key) const {
    return blendStates.at(key);
}

void AppState::CreateDefaultMeshes() {
    LoadMesh("zulu.glb");
    LoadMesh("crate_medium.glb");
}

void AppState::CreateDepthTexture() {
    if (depthTexture != nullptr) {
        SDL_Log("Freeing depth texture...");
        SDL_WaitForGPUIdle(device);
        SDL_ReleaseGPUTexture(device, depthTexture);
        depthTexture = nullptr;
    }
    SDL_Log("Creating depth texture...");
    const SDL_GPUTextureCreateInfo depthInfo = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = static_cast<Uint32>(windowWidth),
        .height = static_cast<Uint32>(windowHeight),
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1
    };
    depthTexture = SDL_CreateGPUTexture(device, &depthInfo);
    if (!depthTexture) {
        SDL_Log("CreateDepthTexture: %s", SDL_GetError());
    }
}

void AppState::CreateLightBuffers() {
    SDL_Log("Creating light buffers...");
    SDL_GPUBufferCreateInfo bufferInfo = {};
    bufferInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
    bufferInfo.size = MAX_LIGHTS * sizeof(PointLight3DGPU);

    lightBuffer = SDL_CreateGPUBuffer(device, &bufferInfo);
    if (!lightBuffer) {
        SDL_Log("Failed to create light buffer: %s", SDL_GetError());
    }

    SDL_GPUTransferBufferCreateInfo transferInfo = {};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = MAX_LIGHTS * sizeof(PointLight3DGPU);

    lightTransferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);
    if (!lightTransferBuffer) {
        SDL_Log("Failed to create light transfer buffer: %s", SDL_GetError());
    }
}

void AppState::CreateDefaultMaterials() {
    SDL_Log("Creating default materials...");
    new MaterialLitTextured(this, "uvs", "UnlitUVs");
    auto* missing_2d = new MaterialLitTextured(this, "missing_2d", "2D");
    missing_2d->setTextureAlbedo(this, "missing.png");
    missing_2d->setSampler(this, "nearest_repeat");
    auto* missing = new MaterialLitTextured(this, "missing", "LitTextured");
    missing->setTextureAlbedo(this, "missing.png");
    missing->setSampler(this, "anisotropic_repeat");
    new MaterialLit(this, "lit", "Lit");
    auto* line = new MaterialLitTextured(this, "line", "Line");
    line->setTextureAlbedo(this, "missing.png");
    line->setSampler(this, "anisotropic_repeat");
    auto* concrete_bricks = new MaterialLitTextured(this, "concrete_bricks", "LitTextured");
    concrete_bricks->setTextureAlbedo(this, "brick_concrete_albedo.png");
    concrete_bricks->setSampler(this, "anisotropic_repeat");
    auto* concrete_bricks_with_specks = new MaterialLitTextured(this, "concrete_bricks_with_specks", "LitTextured");
    concrete_bricks_with_specks->setTextureAlbedo(this, "brick_concrete_specks_albedo.png");
    concrete_bricks_with_specks->setSampler(this, "anisotropic_repeat");
    new MaterialLitTextured(this, "plaster", "LitTextured");
    auto* reinforced_glass = new MaterialLitTextured(this, "reinforced_glass", "LitTextured");
    reinforced_glass->setTextureAlbedo(this, "reinforced_glass_albedo.png");
    reinforced_glass->setSampler(this, "anisotropic_repeat");
    auto* fence = new MaterialLitTextured(this, "fence", "UnlitTexturedAlpha");
    fence->setTextureAlbedo(this, "fence_albedo.png");
    fence->setSampler(this, "anisotropic_repeat");
    auto* asphalt = new MaterialLitTextured(this, "asphalt", "LitTextured");
    asphalt->setTextureAlbedo(this, "asphalt_albedo.png");
    asphalt->setSampler(this, "anisotropic_repeat");
    auto* asphalt_2 = new MaterialLitTextured(this, "asphalt_2", "LitTextured");
    asphalt_2->setTextureAlbedo(this, "asphalt_2_albedo.png");
    asphalt_2->setSampler(this, "anisotropic_repeat");
    auto* concrete = new MaterialLitTextured(this, "concrete", "LitTextured");
    concrete->setTextureAlbedo(this, "concrete_albedo.png");
    concrete->setSampler(this, "anisotropic_repeat");
    auto* concrete_with_specks = new MaterialLitTextured(this, "concrete_with_specks", "LitTextured");
    concrete_with_specks->setTextureAlbedo(this, "concrete_specks_albedo.png");
    concrete_with_specks->setSampler(this, "anisotropic_repeat");
    auto* hardwood_dark = new MaterialLitTextured(this, "hardwood_dark", "LitTextured");
    hardwood_dark->setTextureAlbedo(this, "hardwood_dark_albedo.png");
    hardwood_dark->setSampler(this, "anisotropic_repeat");
    auto* hardwood_light = new MaterialLitTextured(this, "hardwood_light", "LitTextured");
    hardwood_light->setTextureAlbedo(this, "hardwood_light_albedo.png");
    hardwood_light->setSampler(this, "anisotropic_repeat");
    auto* pine_end = new MaterialLitTextured(this, "pine_end", "LitTextured");
    pine_end->setTextureAlbedo(this, "pine_end_albedo.png");
    pine_end->setSampler(this, "anisotropic_repeat");
    auto* pine_wood_dark = new MaterialLitTextured(this, "pine_wood_dark", "LitTextured");
    pine_wood_dark->setTextureAlbedo(this, "pine_wood_dark_albedo.png");
    pine_wood_dark->setSampler(this, "anisotropic_repeat");
    auto* pine_wood_light = new MaterialLitTextured(this, "pine_wood_light", "LitTextured");
    pine_wood_light->setTextureAlbedo(this, "pine_wood_light_albedo.png");
    pine_wood_light->setSampler(this, "anisotropic_repeat");
    auto* roof_tile = new MaterialLitTextured(this, "roof_tile", "LitTextured");
    roof_tile->setTextureAlbedo(this, "roof_tile_albedo.png");
    roof_tile->setSampler(this, "anisotropic_repeat");
    auto* wood_plank = new MaterialLitTextured(this, "wood_plank", "LitTextured");
    wood_plank->setTextureAlbedo(this, "wood_plank_albedo.png");
    wood_plank->setSampler(this, "anisotropic_repeat");
    auto* grid_grey = new MaterialLitTextured(this, "grid_grey", "LitTextured");
    grid_grey->setTextureAlbedo(this, "grid_grey_albedo.png");
    grid_grey->setSampler(this, "anisotropic_repeat");
    auto* grid_orange = new MaterialLitTextured(this, "grid_orange", "LitTextured");
    grid_orange->setTextureAlbedo(this, "grid_orange_albedo.png");
    grid_orange->setSampler(this, "anisotropic_repeat");
    auto* paint_red = new MaterialLitTextured(this, "paint_red", "LitTextured");
    paint_red->setColorAlbedo(glm::vec4(0.5f, 0.25f, 0.25f, 1.0f));
    auto* paint_beige = new MaterialLitTextured(this, "paint_beige", "LitTextured");
    paint_beige->setColorAlbedo(glm::vec4(0.75f, 0.55f, 0.45f, 1.0f));
    auto* clip = new MaterialLitTextured(this, "clip", "UnlitTexturedAlpha");
    clip->setColorAlbedo(glm::vec4(1.0f, 0.0f, 0.0f, 0.5f));
    auto* glass = new MaterialLitTextured(this, "glass", "UnlitTexturedAlpha");
    glass->setColorAlbedo(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    auto* metal_silver = new MaterialLitTextured(this, "metal_silver", "LitTextured");
    metal_silver->setColorAlbedo(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    auto* blend_brick_concrete = new MaterialLitTextured(this, "blend_brick_concrete", "LitTextured");
    blend_brick_concrete->setColorAlbedo(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
}

void AppState::CreateDefaultSamplers() {
    SDL_Log("Creating default samplers...");
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

    constexpr SDL_GPUSamplerCreateInfo nearestSamplerInfo = {
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
    if (SDL_GPUSampler* nearestSampler = SDL_CreateGPUSampler(device, &nearestSamplerInfo)) {
        samplers["nmearest_clamp"] = nearestSampler;
    } else {
        SDL_Log("Couldn't create nearest sampler: %s", SDL_GetError());
    }

    constexpr SDL_GPUSamplerCreateInfo nearestRepeatInfo = {
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
    if (SDL_GPUSampler* nearestRepeatSampler = SDL_CreateGPUSampler(device, &nearestRepeatInfo)) {
        samplers["nearest_repeat"] = nearestRepeatSampler;
    } else {
        SDL_Log("Couldn't create nearest repeat sampler: %s", SDL_GetError());
    }

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

    CreatePipeline("Line", "Default", "UnlitTextured", "Line", "Default", true, true);
    CreatePipeline("UnlitTextured", "Default", "UnlitTextured", "Fill", "Default", true, true);
    CreatePipeline("UnlitTexturedAlpha", "Default", "UnlitTextured", "Fill", "Alpha", true, false);
    CreatePipeline("UnlitUVs", "Default", "UnlitUVs", "Fill", "Default", true, true);
    CreatePipeline("2D", "Default", "UnlitTextured", "FillNoBack", "Alpha", false, false);
    CreatePipeline("Lit", "Default", "Lit", "Fill", "Default", true, true);
    CreatePipeline("LitTextured", "Default", "LitTextured", "Fill", "Default", true, true);
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
    rasterizerStates.insert_or_assign("FillNoBack", fillNoBack);

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

void AppState::CreateDefaultMultisampleStates()
{
    SDL_GPUMultisampleState defaultMultisampleState = {};
    defaultMultisampleState.sample_count = SDL_GPU_SAMPLECOUNT_1;
    defaultMultisampleState.enable_mask = false;
    defaultMultisampleState.enable_alpha_to_coverage = true;
    defaultMultisampleState.sample_mask = 0;
    multisampleStates.insert_or_assign("Multisample", defaultMultisampleState);
}