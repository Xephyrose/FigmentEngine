#include "MaterialLitTextured.h"
#include <SDL3/SDL_log.h>

void MaterialLitTextured::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer) {
    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    SDL_GPUTexture* getAlbedo = appState->GetTexture(textureAlbedo);
    SDL_GPUSampler* getSampler = appState->GetSampler(samplerAlbedo);

    const SDL_GPUTextureSamplerBinding binding = {getAlbedo, getSampler};
    SDL_BindGPUFragmentSamplers(appState->renderPass, 0, &binding, 1);

    struct PushData {
        glm::vec4 colorAlbedo;
        bool      useTexture;
    };
    PushData push{};
    push.colorAlbedo = this->colorAlbedo;
    push.useTexture = (this->textureAlbedo == "none" ? 0 : 1);

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &push, sizeof(PushData));

    SDL_BindGPUFragmentStorageBuffers(
        appState->renderPass,
        0,
        &appState->lightBuffer,
        1
    );
}
