#include "AppState.h"

#include <filesystem>
#include <SDL3_image/SDL_image.h>

#include "MaterialUnlitTextured.h"
#include "Vertex.h"

bool AppState::CreatePipeline(const std::string& name, const std::string& vertShader, const std::string& fragShader) {
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
            .format = SDL_GetGPUSwapchainTextureFormat(device, window)
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
        .rasterizer_state = SDL_GPURasterizerState{
            .fill_mode = SDL_GPU_FILLMODE_FILL,
            .cull_mode = SDL_GPU_CULLMODE_BACK,
            .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
        },
        .depth_stencil_state = SDL_GPUDepthStencilState{
            .compare_op = SDL_GPU_COMPAREOP_LESS,
            .enable_depth_test = true,
            .enable_depth_write = true,
        },
        .target_info = SDL_GPUGraphicsPipelineTargetInfo{
            .color_target_descriptions = colorTargetDescriptions.data(),
            .num_color_targets = colorTargetDescriptions.size(),
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
            .has_depth_stencil_target = true
        },
    };

    SDL_Log("Created pipeline with key %s", name.c_str());
    pipelines.insert_or_assign(name, SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo));
    if (!pipelines[name]) {
        SDL_Log("Couldn't create graphics pipeline: %s", SDL_GetError());
        return false;
    }

    SDL_ReleaseGPUShader(device, vertexShader);
    SDL_ReleaseGPUShader(device, fragmentShader);
    return true;
}

bool AppState::LoadTexture(std::string path) {
    std::string fullPath = std::filesystem::path(SDL_GetBasePath()) / "assets" / "textures" / path;
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

bool AppState::LoadShader(std::string path) {
    const std::string fullPath = std::filesystem::path(SDL_GetBasePath()) / "assets" / "shaders" / path;
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
        .num_uniform_buffers = stage == SDL_GPU_SHADERSTAGE_VERTEX ? 1u : 0u,
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

SDL_GPUShader* AppState::GetShader(const std::string& path) {
    if (!shaders.contains(path)) {
        if (!LoadShader(path)) {
            SDL_Log("Couldn't load shader %s: %s", path.c_str(), SDL_GetError());
            return nullptr;
        }
    }
    return shaders.at(path);
}

SDL_GPUTexture *AppState::GetTexture(const std::string &path) {

    if (!textures.contains(path)) {
        if (LoadTexture(path) == false) {
            SDL_Log("Couldn't load shader %s: %s", path.c_str(), SDL_GetError());
            return nullptr;
        }
    }
    return textures.at(path);
}

Material *AppState::GetMaterial(const std::string &key) const {
    if (!materials.contains(key)) {
        return nullptr;
    }
    return materials.at(key);
}

SDL_GPUGraphicsPipeline* AppState::GetPipeline(const std::string& type) const {
    return pipelines.at(type);
}

SDL_GPUSampler* AppState::GetSampler(const std::string& type) const {
    return samplers.at(type);
}

void AppState::CreateDefaultMaterials() {
    // concrete_bricks
    GetTexture("concrete_bricks.png");
    auto* material = new MaterialUnlitTextured("concrete_bricks", "UnlitTextured", "anisotropic_repeat", "concrete_bricks.png");
    materials.insert_or_assign("concrete_bricks", material);
}

void AppState::CreateDefaultPipelines() {
    CreatePipeline("UnlitTextured", "UnlitTextured", "UnlitTextured");
    CreatePipeline("UnlitUVs", "UnlitTextured", "UnlitUVs");
}

void AppState::CreateDefaultSamplers() {
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

bool AppState::CreateDefaultTextures() {
    return LoadTexture("missing.png");
}