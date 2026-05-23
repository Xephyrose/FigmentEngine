#include "AppState.h"

#include <filesystem>
#include <SDL3_image/SDL_image.h>

bool AppState::LoadTexture(const std::string& texturePath) {
    if (texture) {
        SDL_ReleaseGPUTexture(device, texture);
        texture = nullptr;
    }
    if (sampler) {
        SDL_ReleaseGPUSampler(device, sampler);
        sampler = nullptr;
    }

    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
    if (!uploadCmdBuf) {
        SDL_Log("Couldn't acquire command buffer: %s", SDL_GetError());
        return false;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    // Load image
    texture = IMG_LoadGPUTexture(device, copyPass, texturePath.c_str(), nullptr, nullptr);

    // End the copy pass
    SDL_EndGPUCopyPass(copyPass);
    // Submit the command buffer
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);

    if (!texture) {
        return false;
    }

    SDL_GPUSamplerCreateInfo samplerInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
    };
    sampler = SDL_CreateGPUSampler(device, &samplerInfo);

    return true;
}

bool AppState::LoadShader(const std::string& shaderFilename) {
    SDL_GPUShaderStage stage;
    if (shaderFilename.contains(".vert"))
    {
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    }
    else if (shaderFilename.contains(".frag"))
    {
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    }
    else
    {
        SDL_Log("Couldn't deduce shader stage from file name: %s", shaderFilename.c_str());
        return false;
    }

    std::filesystem::path fullPath = std::filesystem::path(SDL_GetBasePath()) / "assets" / "shaders";
    // Starts as invalid so we don't assume
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    // Different shaer formats have different entrypoint names
    const char* entrypoint;

    SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
    if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV)
    {
        fullPath /= shaderFilename + ".spv";
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entrypoint = "main";
    }
    else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL)
    {
        fullPath /= shaderFilename + ".msl";
        format = SDL_GPU_SHADERFORMAT_MSL;
        entrypoint = "main0";
    }
    else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL)
    {
        fullPath /= shaderFilename + ".dxil";
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
    void* code = SDL_LoadFile(fullPath.string().c_str(), &fileSize);
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

    shaders.insert_or_assign(shaderFilename, shader);

    return true;
}

SDL_GPUShader* AppState::GetShader(const std::string& shaderFilename) {
    if (!shaders.contains(shaderFilename)) {
        if (LoadShader(shaderFilename) == false) {
            SDL_Log("Couldn't load shader %s: %s", shaderFilename.c_str(), SDL_GetError());
            return nullptr;
        }
    }
    return shaders.at(shaderFilename);
}