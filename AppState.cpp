#include "AppState.h"

#include <SDL3_image/SDL_image.h>

bool AppState::LoadTextureFromFile(const std::string& texturePath) {
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
