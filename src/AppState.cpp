#include "AppState.h"

#include <thirdparty/tiny_gltf.h>
#include <filesystem>
#include <SDL3_image/SDL_image.h>

#include "Camera3D.h"
#include "MaterialPhong.h"
#include "MaterialPhongTextured.h"
#include "MaterialUnlitTextured.h"
#include "Mesh.h"
#include "Light3DGPU.h"
#include "Material2D.h"
#include "MaterialColor.h"
#include "MaterialHeightMap.h"
#include "MaterialPBR.h"
#include "MaterialPBRORM.h"
#include "Vertex.h"
#include "SDL3/SDL_log.h"
#include "thirdparty/json.hpp"

AppState::~AppState() {
    for (const auto &material: materials | std::views::values) {delete material;}
    materials.clear();

    for (const auto &texture: textures | std::views::values) {
        if (texture) {
            SDL_ReleaseGPUTexture(device, texture);
        }
    }
    textures.clear();
    SDL_ReleaseGPUTexture(device, depthTexture);
    SDL_ReleaseGPUTexture(device, shadowMap);
    SDL_ReleaseGPUTexture(device, msaaColorTarget);

    for (const auto &surface: surfaces | std::views::values) {
        if (surface) {
            SDL_DestroySurface(surface);
        }
    }
    textures.clear();

    for (const auto &sampler: samplers | std::views::values) {
        if (sampler) {
            SDL_ReleaseGPUSampler(device, sampler);
        }
    }
    samplers.clear();

    for (auto &mesh: meshes | std::views::values) {
        mesh.ReleaseGPUResources(this);
    }
    meshes.clear();
    quadMesh->ReleaseGPUResources(this);

    for (const auto &pipeline: pipelines | std::views::values) {
        if (pipeline) {
            SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
        }
    }
    pipelines.clear();
    SDL_ReleaseGPUGraphicsPipeline(device, shadowPipeline);

    for (const auto &shader: shaders | std::views::values) {
        if (shader) {
            SDL_ReleaseGPUShader(device, shader);
        }
    }
    shaders.clear();

    SDL_ReleaseGPUBuffer(device, pointLightBuffer);
    SDL_ReleaseGPUBuffer(device, directionalLightBuffer);
    SDL_ReleaseGPUBuffer(device, spotLightBuffer);

    SDL_ReleaseGPUTransferBuffer(device, pointLightTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(device, directionalLightTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(device, spotLightTransferBuffer);

    b2DestroyWorld(worldId2);
    b3DestroyWorld(worldId3);

    SDL_ReleaseWindowFromGPUDevice(device, window);
    // hehehe kill rog astral 5090 with hammers
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
}

void AppState::updatePhysicsTimeStep() {
    fixedTimeStep = 1.0f / static_cast<float>(physics_tps);
}

SDL_Surface* AppState::DownloadGPUTexture(SDL_GPUTexture *texture) {
    SDL_GPUTransferBufferCreateInfo createInfo{};
    createInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    createInfo.size = 4096 * 4096 * 4;
    SDL_GPUTransferBuffer* tbuffer = SDL_CreateGPUTransferBuffer(device, &createInfo);
    SDL_GPUCommandBuffer* cbuffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* cpass = SDL_BeginGPUCopyPass(cbuffer);

    const SDL_GPUTextureRegion region{texture, 0, 0, 0, 0, 0, 4096, 4096, 1};
    const SDL_GPUTextureTransferInfo destination{tbuffer, 0, 0, 0};

    SDL_DownloadFromGPUTexture(cpass, &region, &destination);
    SDL_EndGPUCopyPass(cpass);

    SDL_SubmitGPUCommandBuffer(cbuffer);
    SDL_WaitForGPUIdle(device);

    const void* mapped_data = SDL_MapGPUTransferBuffer(device, tbuffer, false);

    // Create a surface that OWNS its own memory (not just referencing the transfer buffer)
    SDL_Surface* surface = SDL_CreateSurface(4096, 4096, SDL_PIXELFORMAT_RGBA32);
    if (surface) {
        // Copy the data from the transfer buffer to the surface's own memory
        SDL_memcpy(surface->pixels, mapped_data, createInfo.size);
    }

    SDL_UnmapGPUTransferBuffer(device, tbuffer);
    SDL_ReleaseGPUTransferBuffer(device, tbuffer);

    return surface;
}

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

    const std::array colorTargetDescriptions{
        SDL_GPUColorTargetDescription{
            .format = SDL_GetGPUSwapchainTextureFormat(device, window),
            .blend_state = GetBlendState(blendState),
        }
    };

    const auto pipelineCreateInfo = SDL_GPUGraphicsPipelineCreateInfo{
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .vertex_input_state = vertexInputState,
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

            auto tanIt = primitive.attributes.find("TANGENT");
            if (tanIt != primitive.attributes.end()) {
                std::vector<float> tangents = ReadAttributeData(model, tanIt->second);
                size_t vertexCount = tangents.size() / 4;
                for (size_t i = 0; i < vertexCount && i < submesh.vertexCount; i++) {
                    result.vertices[startVertex + i].tangent = glm::vec4(
                        tangents[i * 4], tangents[i * 4 + 1], tangents[i * 4 + 2], tangents[i * 4 + 3]
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
    SDL_Log("Loading shader %s...", path.c_str());
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
    if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL)
    {
        extension = ".dxil";
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entrypoint = "main";
    }
    else if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV)
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
    else
    {
        SDL_Log("Couldn't find a supported shader format for backend %s!", SDL_GetGPUDeviceDriver(device));
        return false;
    }

    size_t fileSize;
    void* code = SDL_LoadFile((fullPath + extension).c_str(), &fileSize);
    if (code == nullptr)
    {
        SDL_Log("Couldn't load shader file from disk!\n\t%s", SDL_GetError());
        return false;
    }

    const std::string jsonPath = fullPath + ".json";
    size_t jsonSize;
    void* jsonData = SDL_LoadFile(jsonPath.c_str(), &jsonSize);
    if (jsonData == nullptr) {
        SDL_Log("Couldn't load shader metadata JSON from disk!\n\t%s", SDL_GetError());
        SDL_free(code);
        return false;
    }

    std::string jsonString(static_cast<char*>(jsonData), jsonSize);
    SDL_free(jsonData);

    try {
        const nlohmann::json metadata = nlohmann::json::parse(jsonString);

        const uint32_t numSamplers = metadata.value("samplers", 0);
        const uint32_t numStorageTextures = metadata.value("storage_textures", 0);
        const uint32_t numStorageBuffers = metadata.value("storage_buffers", 0);
        const uint32_t numUniformBuffers = metadata.value("uniform_buffers", 0);

        SDL_Log("Creating shader %s, num_samplers is %u, num_storage_textures is %u, num_storage_buffers is %u, num_uniform_buffers is %u",
                path.c_str(), numSamplers, numStorageTextures, numStorageBuffers, numUniformBuffers);

        const auto shaderInfo = SDL_GPUShaderCreateInfo{
            .code_size = fileSize,
            .code = static_cast<Uint8*>(code),
            .entrypoint = entrypoint,
            .format = format,
            .stage = stage,
            .num_samplers = numSamplers,
            .num_storage_textures = numStorageTextures,
            .num_storage_buffers = numStorageBuffers,
            .num_uniform_buffers = numUniformBuffers,
        };

        SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
        if (shader == nullptr) {
            SDL_Log("Couldn't create shader from file %s: %s", fullPath.c_str(), SDL_GetError());
            SDL_free(code);
            return false;
        }

        shaders.insert_or_assign(path, shader);
        SDL_free(code);
        return true;

    } catch (const nlohmann::json::parse_error& e) {
        SDL_Log("Failed to parse shader metadata JSON: %s", e.what());
        SDL_free(code);
        return false;
    }
}

bool AppState::LoadTexture(const std::string& path, const SDL_PixelFormat preferred_format) {
    const std::string fullPath = (std::filesystem::path(SDL_GetBasePath()) / "assets" / "textures" / path).string();

    // 1. Load the image surface
    SDL_Surface* surface = IMG_Load(fullPath.c_str());
    if (!surface) {
        SDL_Log("Couldn't load image: %s", SDL_GetError());
        return false;
    }

    // Convert to RGBA32 (SDL_ConvertSurface doesn't take a third argument anymore)
    SDL_Surface* converted = SDL_ConvertSurface(surface, preferred_format);
    SDL_DestroySurface(surface);
    if (!converted) {
        SDL_Log("Failed to convert surface format: %s", SDL_GetError());
        return false;
    }

    // 2. Acquire command buffer and begin copy pass
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
    if (!uploadCmdBuf) {
        SDL_Log("Couldn't acquire command buffer: %s", SDL_GetError());
        SDL_DestroySurface(converted);
        return false;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
    if (!copyPass) {
        SDL_Log("Couldn't begin copy pass: %s", SDL_GetError());
        SDL_DestroySurface(converted);
        SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
        return false;
    }

    int mipLevels = 1;
    int maxDimension = std::max(converted->w, converted->h);
    while (maxDimension > 1) {
        maxDimension /= 2;
        mipLevels++;
    }

    SDL_GPUTextureFormat format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    if (fullPath.contains("albedo")) format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB;

    // 3. Create the texture with num_levels = 0 (auto-generate all mip levels)
    SDL_GPUTextureCreateInfo textureInfo = {};
    textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
    textureInfo.format = format;
    textureInfo.width = converted->w;
    textureInfo.height = converted->h;
    textureInfo.layer_count_or_depth = 1;
    textureInfo.num_levels = mipLevels;  // Generate all mip levels
    textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    textureInfo.props = 0;

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &textureInfo);
    if (!texture) {
        SDL_Log("Couldn't create texture: %s", SDL_GetError());
        SDL_DestroySurface(converted);
        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
        return false;
    }

    // 4. Create a transfer buffer for the pixel data
    const size_t pixelDataSize = converted->h * converted->pitch;
    SDL_GPUTransferBufferCreateInfo transferInfo = {};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = pixelDataSize;
    transferInfo.props = 0;

    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);
    if (!transferBuffer) {
        SDL_Log("Couldn't create transfer buffer: %s", SDL_GetError());
        SDL_DestroySurface(converted);
        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
        return false;
    }

    // 5. Map the transfer buffer and copy the pixel data
    void* mapped = SDL_MapGPUTransferBuffer(device, transferBuffer, false);
    if (!mapped) {
        SDL_Log("Couldn't map transfer buffer: %s", SDL_GetError());
        SDL_DestroySurface(converted);
        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
        return false;
    }

    memcpy(mapped, converted->pixels, pixelDataSize);
    SDL_UnmapGPUTransferBuffer(device, transferBuffer);

    // 6. Upload from transfer buffer to texture
    SDL_GPUTextureTransferInfo uploadInfo = {};
    uploadInfo.transfer_buffer = transferBuffer;
    uploadInfo.offset = 0;
    uploadInfo.pixels_per_row = converted->pitch / 4;  // RGBA32 = 4 bytes per pixel
    uploadInfo.rows_per_layer = converted->h;

    SDL_GPUTextureRegion region = {};
    region.texture = texture;
    region.x = 0;
    region.y = 0;
    region.w = converted->w;
    region.h = converted->h;
    region.d = 1;
    region.mip_level = 0;
    region.layer = 0;

    SDL_UploadToGPUTexture(copyPass, &uploadInfo, &region, false);

    // 7. End the copy pass and submit
    SDL_EndGPUCopyPass(copyPass);

    // 2. Submit the upload command buffer and wait for it to complete
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(uploadCmdBuf);
    if (!fence) {
        SDL_Log("Couldn't acquire fence: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
        return false;
    }
    SDL_WaitForGPUFences(device, true, &fence, 1);
    SDL_ReleaseGPUFence(device, fence);

    // 3. Release the transfer buffer (upload is complete)
    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

    // 4. Acquire a NEW command buffer for mipmap generation
    SDL_GPUCommandBuffer* mipCmdBuf = SDL_AcquireGPUCommandBuffer(device);
    if (!mipCmdBuf) {
        SDL_Log("Couldn't acquire command buffer for mipmap generation");
        return false;
    }

    // 5. Generate mipmaps using the new command buffer
    SDL_GenerateMipmapsForGPUTexture(mipCmdBuf, texture);

    // 6. Submit the mipmap command buffer
    SDL_GPUFence* mipFence = SDL_SubmitGPUCommandBufferAndAcquireFence(mipCmdBuf);
    if (mipFence) {
        SDL_WaitForGPUFences(device, true, &mipFence, 1);
        SDL_ReleaseGPUFence(device, mipFence);
    }

    // 7. Store the texture
    textures.insert_or_assign(path, texture);
    surfaces.insert_or_assign(path, converted);

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
        SDL_Log("Couldn't find material %s", key.c_str());
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
            SDL_Log("Couldn't load texture %s: %s", path.c_str(), SDL_GetError());
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

void AppState::CreateVertexinputState() {
    m_vertexBufferDescriptions[0] = {
        SDL_GPUVertexBufferDescription{
            .slot = 0,
            .pitch = sizeof(Vertex),
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
        },
    };

    m_vertexAttributes[0] = SDL_GPUVertexAttribute{
        .location = 0,
        .buffer_slot = 0,
        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
        .offset = offsetof(Vertex, position),
    },
    m_vertexAttributes[1] = SDL_GPUVertexAttribute{
        .location = 1,
        .buffer_slot = 0,
        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
        .offset = offsetof(Vertex, uv),
    },
    m_vertexAttributes[2] = SDL_GPUVertexAttribute{
        .location = 2,
        .buffer_slot = 0,
        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
        .offset = offsetof(Vertex, normal),
    },
    m_vertexAttributes[3] = SDL_GPUVertexAttribute{
        .location = 3,
        .buffer_slot = 0,
        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
        .offset = offsetof(Vertex, tangent),
    };

    vertexInputState.vertex_buffer_descriptions = m_vertexBufferDescriptions.data();
    vertexInputState.num_vertex_buffers = m_vertexBufferDescriptions.size();
    vertexInputState.vertex_attributes = m_vertexAttributes.data();
    vertexInputState.num_vertex_attributes = m_vertexAttributes.size();
}

SDL_GPUColorTargetBlendState AppState::GetBlendState(const std::string &key) const {
    return blendStates.at(key);
}

void AppState::CreateDefaultMeshes() {
    LoadMesh("zoo.glb");
    LoadMesh("crate_medium.glb");
    LoadMesh("subdivided_plane.glb");
}

void AppState::CreateDepthTexture() {
    if (depthTexture != nullptr) {
        SDL_Log("Freeing depth texture...");
        SDL_WaitForGPUIdle(device);
        SDL_ReleaseGPUTexture(device, depthTexture);
        depthTexture = nullptr;
    }
    SDL_Log("Creating depth texture with sample count %d...", msaaSamples);
    const SDL_GPUTextureCreateInfo depthInfo = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = static_cast<Uint32>(windowWidth),
        .height = static_cast<Uint32>(windowHeight),
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = static_cast<SDL_GPUSampleCount>(msaaSamples)
    };
    depthTexture = SDL_CreateGPUTexture(device, &depthInfo);
    if (!depthTexture) {
        SDL_Log("CreateDepthTexture: %s", SDL_GetError());
    }
}

void AppState::CreateMSAAColorTarget() {
    if (msaaColorTarget != nullptr) {
        SDL_Log("Freeing MSAA color target...");
        SDL_WaitForGPUIdle(device);
        SDL_ReleaseGPUTexture(device, msaaColorTarget);
        msaaColorTarget = nullptr;
    }
    SDL_Log("Creating MSAA color target with sample count %d...", msaaSamples);
    const SDL_GPUTextureCreateInfo colorInfo = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GetGPUSwapchainTextureFormat(device, window),
        .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
        .width = static_cast<Uint32>(windowWidth),
        .height = static_cast<Uint32>(windowHeight),
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = static_cast<SDL_GPUSampleCount>(msaaSamples)
    };
    msaaColorTarget = SDL_CreateGPUTexture(device, &colorInfo);
    if (!msaaColorTarget) {
        SDL_Log("CreateMSAAColorTarget: %s", SDL_GetError());
    }
}

void AppState::CreatePointLightBuffer() {
    SDL_Log("Creating point light buffer of size %i", pointLights.size());
    if (pointLightBuffer != nullptr) {
        SDL_Log("Freeing point light buffer...");
        SDL_WaitForGPUIdle(device);
        SDL_ReleaseGPUBuffer(device, pointLightBuffer);
    }
    SDL_GPUBufferCreateInfo pointBufferInfo = {};
    pointBufferInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
    pointBufferInfo.size = std::max(pointLights.size() * sizeof(PointLight3DGPU), sizeof(uint32_t)); // slightly hacky but necessary, buffers must be at least 4 bytes

    pointLightBuffer = SDL_CreateGPUBuffer(device, &pointBufferInfo);
    if (!pointLightBuffer) {
        SDL_Log("Failed to create point light buffer: %s", SDL_GetError());
    }

    SDL_GPUTransferBufferCreateInfo pointTransferInfo = {};
    pointTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    pointTransferInfo.size = std::max(pointLights.size() * sizeof(PointLight3DGPU), sizeof(uint32_t));

    pointLightTransferBuffer = SDL_CreateGPUTransferBuffer(device, &pointTransferInfo);
    if (!pointLightTransferBuffer) {
        SDL_Log("Failed to create light transfer buffer: %s", SDL_GetError());
    }
}

void AppState::CreateDirectionalLightBuffer() {
    SDL_Log("Creating directional light buffer of size %i", directionalLights.size());

    if (directionalLightBuffer != nullptr) {
        SDL_Log("Freeing directional light buffer...");
        SDL_WaitForGPUIdle(device);
        SDL_ReleaseGPUBuffer(device, directionalLightBuffer);
    }
    SDL_GPUBufferCreateInfo directionalBufferInfo = {};
    directionalBufferInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
    directionalBufferInfo.size = std::max(directionalLights.size() * sizeof(DirectionalLight3DGPU), sizeof(uint32_t));

    directionalLightBuffer = SDL_CreateGPUBuffer(device, &directionalBufferInfo);
    if (!directionalLightBuffer) {
        SDL_Log("Failed to create directional light buffer: %s", SDL_GetError());
    }

    SDL_GPUTransferBufferCreateInfo directionalTransferInfo = {};
    directionalTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    directionalTransferInfo.size = std::max(directionalLights.size() * sizeof(DirectionalLight3DGPU), sizeof(uint32_t));

    directionalLightTransferBuffer = SDL_CreateGPUTransferBuffer(device, &directionalTransferInfo);
    if (!directionalLightTransferBuffer) {
        SDL_Log("Failed to create directional light transfer buffer: %s", SDL_GetError());
    }
}

void AppState::CreateSpotLightBuffer() {
    SDL_Log("Creating spot light buffer of size %i", pointLights.size());

    if (spotLightBuffer != nullptr) {
        SDL_Log("Freeing spot light buffer...");
        SDL_WaitForGPUIdle(device);
        SDL_ReleaseGPUBuffer(device, spotLightBuffer);
    }
    SDL_GPUBufferCreateInfo spotBufferInfo = {};
    spotBufferInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
    spotBufferInfo.size = std::max(spotLights.size() * sizeof(SpotLight3DGPU), sizeof(uint32_t));

    spotLightBuffer = SDL_CreateGPUBuffer(device, &spotBufferInfo);
    if (!spotLightBuffer) {
        SDL_Log("Failed to create spot light buffer: %s", SDL_GetError());
    }

    SDL_GPUTransferBufferCreateInfo spotTransferInfo = {};
    spotTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    spotTransferInfo.size = std::max(spotLights.size() * sizeof(SpotLight3DGPU), sizeof(uint32_t));

    spotLightTransferBuffer = SDL_CreateGPUTransferBuffer(device, &spotTransferInfo);
    if (!spotLightTransferBuffer) {
        SDL_Log("Failed to create spot light transfer buffer: %s", SDL_GetError());
    }
}

void AppState::CreateDefaultMaterials() {
    SDL_Log("Creating default materials...");
    auto* missing_2d = new Material2D(this, "missing_2d", "2D");
    missing_2d->setTextureAlbedo(this, "missing.png");
    missing_2d->setSampler(this, "nearest_repeat");

    auto* missing = new MaterialUnlitTextured(this, "missing", "UnlitTextured");
    missing->setTextureAlbedo(this, "missing.png");
    missing->setSampler(this, "anisotropic_repeat");

    new MaterialPhong(this, "phong", "Phong");
    const auto phong_tex = new MaterialPhongTextured(this, "phong_textured", "PhongTextured");
    phong_tex->setTextureAlbedo(this, "missing.png");
    phong_tex->setSampler(this, "nearest_repeat");
    new MaterialPhong(this, "blinn_phong", "BlinnPhong");
    const auto blinn_phong_tex = new MaterialPhongTextured(this, "blinn_phong_textured", "BlinnPhongTextured");
    blinn_phong_tex->setTextureAlbedo(this, "missing.png");
    blinn_phong_tex->setSampler(this, "nearest_repeat");

    auto* line = new MaterialColor(this, "line", "Line");
    line->setColor(glm::vec4(1, 0, 1, 1));

    auto* pbr = new MaterialPBR(this, "pbr", "PBR");
    pbr->setColorAlbedo(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    auto* pbr_orm = new MaterialPBRORM(this, "pbr_orm", "PBRORM");
    pbr_orm->setColorAlbedo(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    pbr_orm->setTextureAlbedo(this, "learnopengl_rustediron2_basecolor.png");
    pbr_orm->setTextureORM(this, "learnopengl_rustediron2_orm.png");
    pbr_orm->setTextureNormalMap(this, "learnopengl_rustediron2_normal.png");

    auto* heightmap = new MaterialHeightMap(this, "heightmap", "HeightMap");
    heightmap->setTextureAlbedo(this, "learnopengl_iceland_height_200_blurred.png");

    auto* concrete_bricks = new MaterialPBRORM(this, "concrete_bricks", "PBRORM");
    concrete_bricks->setTextureAlbedo(this, "brick_concrete_albedo.png");
    concrete_bricks->setSampler(this, "anisotropic_repeat");
    concrete_bricks->setTextureNormalMap(this, "brick_concrete_normal.png");

    auto* concrete_bricks_with_specks = new MaterialPBRORM(this, "concrete_bricks_with_specks", "PBRORM");
    concrete_bricks_with_specks->setTextureAlbedo(this, "brick_concrete_specks_albedo.png");
    concrete_bricks_with_specks->setSampler(this, "anisotropic_repeat");
    concrete_bricks_with_specks->setTextureNormalMap(this, "brick_concrete_normal.png");

    auto* plaster = new MaterialPBRORM(this, "plaster", "PBRORM");
    plaster->setSampler(this, "anisotropic_repeat");
    plaster->setTextureNormalMap(this, "plaster_normal.png");
    plaster->setColorMetallic(0.13f);
    plaster->setColorRoughness(0.5f);

    auto* reinforced_glass = new MaterialPBRORM(this, "reinforced_glass", "PBRORM");
    reinforced_glass->setTextureAlbedo(this, "reinforced_glass_albedo.png");
    reinforced_glass->setSampler(this, "anisotropic_repeat");
    reinforced_glass->setTextureNormalMap(this, "reinforced_glass_normal.png");

    auto* fence = new MaterialPBRORM(this, "fence", "PBRORM");
    fence->setTextureAlbedo(this, "fence_albedo.png");
    fence->setSampler(this, "anisotropic_repeat");
    fence->setTextureNormalMap(this, "fence_normal.png");

    auto* asphalt = new MaterialPBRORM(this, "asphalt", "PBRORM");
    asphalt->setTextureAlbedo(this, "asphalt_albedo.png");
    asphalt->setSampler(this, "anisotropic_repeat");
    asphalt->setTextureNormalMap(this, "asphalt_normal.png");
    asphalt->setColorRoughness(0.5f);

    auto* asphalt_2 = new MaterialPBRORM(this, "asphalt_2", "PBRORM");
    asphalt_2->setTextureAlbedo(this, "asphalt_2_albedo.png");
    asphalt_2->setSampler(this, "anisotropic_repeat");

    auto* concrete = new MaterialPBRORM(this, "concrete", "PBRORM");
    concrete->setTextureAlbedo(this, "concrete_albedo.png");
    concrete->setSampler(this, "anisotropic_repeat");

    auto* concrete_with_specks = new MaterialPBRORM(this, "concrete_with_specks", "PBRORM");
    concrete_with_specks->setTextureAlbedo(this, "concrete_specks_albedo.png");
    concrete_with_specks->setSampler(this, "anisotropic_repeat");

    auto* hardwood_dark = new MaterialPBRORM(this, "hardwood_dark", "PBRORM");
    hardwood_dark->setTextureAlbedo(this, "hardwood_dark_albedo.png");
    hardwood_dark->setSampler(this, "anisotropic_repeat");
    hardwood_dark->setTextureNormalMap(this, "hardwood_dark_normal.png");

    auto* hardwood_light = new MaterialPBRORM(this, "hardwood_light", "PBRORM");
    hardwood_light->setTextureAlbedo(this, "hardwood_light_albedo.png");
    hardwood_light->setSampler(this, "anisotropic_repeat");
    hardwood_light->setTextureNormalMap(this, "hardwood_light_normal.png");

    auto* pine_end = new MaterialPBRORM(this, "pine_end", "PBRORM");
    pine_end->setTextureAlbedo(this, "pine_end_albedo.png");
    pine_end->setSampler(this, "anisotropic_repeat");
    pine_end->setTextureNormalMap(this, "pine_end_normal.png");

    auto* pine_wood_dark = new MaterialPBRORM(this, "pine_wood_dark", "PBRORM");
    pine_wood_dark->setTextureAlbedo(this, "pine_wood_dark_albedo.png");
    pine_wood_dark->setSampler(this, "anisotropic_repeat");
    pine_wood_dark->setTextureNormalMap(this, "pine_wood_dark_normal.png");

    auto* pine_wood_light = new MaterialPBRORM(this, "pine_wood_light", "PBRORM");
    pine_wood_light->setTextureAlbedo(this, "pine_wood_light_albedo.png");
    pine_wood_light->setSampler(this, "anisotropic_repeat");
    pine_wood_light->setTextureNormalMap(this, "pine_wood_light_normal.png");

    auto* roof_tile = new MaterialPBRORM(this, "roof_tile", "PBRORM");
    roof_tile->setTextureAlbedo(this, "roof_tile_albedo.png");
    roof_tile->setSampler(this, "anisotropic_repeat");
    roof_tile->setTextureNormalMap(this, "roof_tile_normal.png");

    auto* wood_plank = new MaterialPBRORM(this, "wood_plank", "PBRORM");
    wood_plank->setTextureAlbedo(this, "wood_plank_albedo.png");
    wood_plank->setSampler(this, "anisotropic_repeat");
    wood_plank->setTextureNormalMap(this, "wood_plank_normal.png");

    auto* grid_grey = new MaterialPBRORM(this, "grid_grey", "PBRORM");
    grid_grey->setTextureAlbedo(this, "grid_grey_albedo.png");
    grid_grey->setSampler(this, "anisotropic_repeat");

    auto* grid_orange = new MaterialPBRORM(this, "grid_orange", "PBRORM");
    grid_orange->setTextureAlbedo(this, "grid_orange_albedo.png");
    grid_orange->setSampler(this, "anisotropic_repeat");

    auto* paint_red = new MaterialPBRORM(this, "paint_red", "PBRORM");
    paint_red->setColorAlbedo(glm::vec4(0.5f, 0.25f, 0.25f, 1.0f));
    paint_red->setTextureNormalMap(this, "plaster_normal.png");
    paint_red->setColorMetallic(0.13f);
    paint_red->setColorRoughness(0.5f);

    auto* paint_beige = new MaterialPBRORM(this, "paint_beige", "PBRORM");
    paint_beige->setColorAlbedo(glm::vec4(0.75f, 0.55f, 0.45f, 1.0f));
    paint_beige->setTextureNormalMap(this, "plaster_normal.png");
    paint_beige->setColorMetallic(0.13f);
    paint_beige->setColorRoughness(0.5f);

    auto* clip = new MaterialPBRORM(this, "clip", "PBRORMAlpha");
    clip->setColorAlbedo(glm::vec4(1.0f, 0.0f, 0.0f, 0.5f));

    auto* glass = new MaterialPBRORM(this, "glass", "PBRORMAlpha");
    glass->setColorAlbedo(glm::vec4(0.0f, 0.0f, 0.0f, 0.8f));

    auto* metal_silver = new MaterialPBRORM(this, "metal_silver", "PBRORM");
    metal_silver->setColorAlbedo(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

    auto* blend_brick_concrete = new MaterialPBRORM(this, "blend_brick_concrete", "PBRORM");
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
        samplers["nearest_clamp"] = nearestSampler;
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

    SDL_GPUSamplerCreateInfo shadowSamplerinfo = {};
    shadowSamplerinfo.min_filter = SDL_GPU_FILTER_LINEAR;
    shadowSamplerinfo.mag_filter = SDL_GPU_FILTER_LINEAR;
    shadowSamplerinfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    shadowSamplerinfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    shadowSamplerinfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    shadowSamplerinfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    shadowSamplerinfo.compare_op = SDL_GPU_COMPAREOP_LESS;


    if (SDL_GPUSampler* shadowSampler = SDL_CreateGPUSampler(device, &shadowSamplerinfo)) {
        samplers["shadow_sampler"] = shadowSampler;
    } else {
        SDL_Log("Couldn't create shadow map sampler: %s", SDL_GetError());
    }
}

void AppState::CreateDefaultTextures() {
    SDL_Log("Creating default textures...");
    LoadTexture("missing.png");
}

void AppState::CreateDefaultPipelines() {
    SDL_Log("Creating default pipelines...");

    CreatePipeline("Line", "Default", "UnlitColor", "Line", "Default", true, true);
    CreatePipeline("2D", "Default", "UnlitTextured", "Fill", "Alpha", false, false);

    CreatePipeline("UnlitTextured", "Default", "UnlitTextured", "FrontFaces", "Default", true, true);
    CreatePipeline("UnlitTexturedAlpha", "Default", "UnlitTextured", "FrontFaces", "Alpha", true, false);

    CreatePipeline("Phong", "Default", "Phong", "FrontFaces", "Default", true, true);
    CreatePipeline("PhongTextured", "Default", "PhongTextured", "FrontFaces", "Default", true, true);
    CreatePipeline("BlinnPhong", "Default", "BlinnPhong", "FrontFaces", "Default", true, true);
    CreatePipeline("BlinnPhongTextured", "Default", "BlinnPhongTextured", "FrontFaces", "Default", true, true);

    CreatePipeline("PhongAlpha", "Default", "Phong", "FrontFaces", "Alpha", true, false);
    CreatePipeline("PhongTexturedAlpha", "Default", "PhongTextured", "FrontFaces", "Alpha", true, false);
    CreatePipeline("BlinnPhongAlpha", "Default", "BlinnPhong", "FrontFaces", "Alpha", true, false);
    CreatePipeline("BlinnPhongTexturedAlpha", "Default", "BlinnPhongTextured", "FrontFaces", "Alpha", true, false);

    CreatePipeline("PBR", "Default", "PBR", "FrontFaces", "Default", true, true);
    CreatePipeline("PBRORM", "Default", "PBRTextured", "FrontFaces", "Default", true, true);

    CreatePipeline("PBRAlpha", "Default", "PBR", "FrontFaces", "Alpha", true, false);
    CreatePipeline("PBRORMAlpha", "Default", "PBRTextured", "FrontFaces", "Alpha", true, false);

    CreatePipeline("HeightMap", "HeightMap", "UnlitColor", "Line", "Default", true, true);
}

void AppState::CreateDefaultRasterizerStates() {
    SDL_Log("Creating default rasterizer states...");
    SDL_GPURasterizerState frontFaces{};
    frontFaces.fill_mode = SDL_GPU_FILLMODE_FILL;
    frontFaces.cull_mode = SDL_GPU_CULLMODE_BACK;
    frontFaces.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    rasterizerStates.insert_or_assign("FrontFaces", frontFaces);

    SDL_GPURasterizerState fill{};
    fill.fill_mode = SDL_GPU_FILLMODE_FILL;
    fill.cull_mode = SDL_GPU_CULLMODE_NONE;
    fill.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    rasterizerStates.insert_or_assign("Fill", fill);

    SDL_GPURasterizerState backFaces{};
    backFaces.fill_mode = SDL_GPU_FILLMODE_FILL;
    backFaces.cull_mode = SDL_GPU_CULLMODE_NONE;
    backFaces.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
    rasterizerStates.insert_or_assign("BackFaces", backFaces);

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
    defaultMultisampleState.sample_count = static_cast<SDL_GPUSampleCount>(msaaSamples);
    defaultMultisampleState.enable_mask = false;
    defaultMultisampleState.enable_alpha_to_coverage = true;
    defaultMultisampleState.sample_mask = 0;
    multisampleStates.insert_or_assign("Multisample", defaultMultisampleState);
}

void AppState::CreateShadowMap() {
    if (shadowMap) {
        SDL_ReleaseGPUTexture(device, shadowMap);
        shadowMap = nullptr;
    }

    constexpr int shadowMapSize = 4096 * 4;  // You can make this configurable

    SDL_GPUTextureCreateInfo info = {};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    info.width = shadowMapSize;
    info.height = shadowMapSize;
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;

    shadowMap = SDL_CreateGPUTexture(device, &info);
    if (!shadowMap) {
        SDL_Log("Failed to create shadow map: %s", SDL_GetError());
    }
}

void AppState::CreateShadowPipeline() {
    SDL_GPUShader* vertexShader = GetShader("ShadowMap.vert");
    SDL_GPUShader* fragmentShader = GetShader("ShadowMap.frag");

    if (!vertexShader || !fragmentShader) {
        SDL_Log("Failed to load shadow shaders");
        return;
    }

    // No color targets — just depth
    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.vertex_shader = vertexShader;
    pipelineInfo.fragment_shader = fragmentShader;
    pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineInfo.rasterizer_state = GetRasterizerState("BackFaces");

    pipelineInfo.depth_stencil_state.enable_depth_test = true;
    pipelineInfo.depth_stencil_state.enable_depth_write = true;
    pipelineInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;

    pipelineInfo.target_info.has_depth_stencil_target = true;
    pipelineInfo.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    pipelineInfo.target_info.num_color_targets = 0;

    pipelineInfo.vertex_input_state = vertexInputState;

    shadowPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
}

glm::mat4 AppState::GetLightViewProjection() const {
    const DirectionalLight3D* light = directionalLights[0];
    if (!light) return {1.0f};

    const auto& transform = light->GetGlobalTransformInterpolated(fixedTimeStepAccumulator / fixedTimeStep);

    const glm::vec3 lightDir = glm::normalize(transform.getForward());
    const glm::vec3 lightPos = transform.position;

    const glm::vec3 target = lightPos + lightDir;

    const glm::mat4 lightView = glm::lookAt(lightPos, target, glm::vec3(0.0f, 1.0f, 0.0f));

    constexpr float orthoSize = 100.0f;

    const glm::mat4 lightProj = glm::ortho(
        -orthoSize,
        orthoSize,
        -orthoSize,
        orthoSize,
        1.0f,
        100.0f
    );

    return lightProj * lightView;
}

glm::mat4 AppState::GetOffsetLightViewProjection() const {
    const DirectionalLight3D* light = directionalLights[0];
    if (!light) return {1.0f};

    const auto& transform = light->GetGlobalTransformInterpolated(fixedTimeStepAccumulator / fixedTimeStep);
    const glm::vec3 cameraPos = current_camera_3d->GetGlobalTransformInterpolated(fixedTimeStepAccumulator / fixedTimeStep).position;
    constexpr float lightDistance = 50.0f;

    const glm::vec3 lightDir = glm::normalize(transform.getForward());
    const glm::vec3 lightPos = cameraPos - lightDir * lightDistance;

    const glm::vec3 target = lightPos + lightDir;

    const glm::mat4 lightView = glm::lookAt(lightPos, target, glm::vec3(0.0f, 1.0f, 0.0f));

    constexpr float orthoSize = 100.0f;

    const glm::mat4 lightProj = glm::ortho(
        -orthoSize,
        orthoSize,
        -orthoSize,
        orthoSize,
        1.0f,
        100.0f
    );

    return lightProj * lightView;
}

void AppState::RecreateAllMultisampleStates() {
    SDL_Log("Recreating all multisample states...");
    multisampleStates.clear();
    CreateDefaultMultisampleStates();
}

void AppState::RecreateAllPipelines() {
    SDL_Log("Recreating all pipelines...");
    RecreateAllMultisampleStates();
    for (const auto &pipeline: pipelines | std::views::values) {
        SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    }
    pipelines.clear();

    CreateDefaultPipelines();
}

void AppState::RenderShadowMap(SDL_GPUCommandBuffer* cmdBuf, const glm::mat4& lightViewProj) {
    if (!shadowMap || !shadowPipeline) return;

    SDL_GPUDepthStencilTargetInfo depthTarget = {};
    depthTarget.texture = shadowMap;
    depthTarget.clear_depth = 1.0f; // Far plane
    depthTarget.load_op = SDL_GPU_LOADOP_CLEAR;
    depthTarget.store_op = SDL_GPU_STOREOP_STORE;
    depthTarget.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
    depthTarget.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
    depthTarget.cycle = true;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmdBuf, nullptr, 0, &depthTarget);
    if (!pass) {
        SDL_Log("Failed to begin shadow render pass");
        return;
    }

    SDL_BindGPUGraphicsPipeline(pass, shadowPipeline);

    // SDL_Log("LightVP: [%f %f %f %f]", lightViewProj[0][0], lightViewProj[0][1], lightViewProj[0][2], lightViewProj[0][3]);
    SDL_PushGPUVertexUniformData(cmdBuf, 0, &lightViewProj, sizeof(lightViewProj));

    root.DrawShadow(*this, cmdBuf, pass);

    SDL_EndGPURenderPass(pass);
}