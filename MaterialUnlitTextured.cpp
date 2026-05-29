#include "MaterialUnlitTextured.h"

#include <SDL3/SDL_log.h>

MaterialUnlitTextured::MaterialUnlitTextured(AppState* appState, const std::string &name, const std::string &pipeline, const std::string& sampler, const std::string &textureAlbedo) {
    this->name = name;
    this->pipeline = pipeline;
    this->sampler = sampler;
    appState->LoadTexture(textureAlbedo);
    this->textureAlbedo = textureAlbedo;
    appState->materials.insert_or_assign(name, this);
}

void MaterialUnlitTextured::Bind(AppState *appState, SDL_GPUCommandBuffer* commandBuffer) const {
    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    SDL_GPUTexture* getAlbedo = appState->GetTexture(textureAlbedo);
    SDL_GPUSampler* getSampler = appState->GetSampler(sampler);

    if (getAlbedo && getSampler) {
        const SDL_GPUTextureSamplerBinding binding = {getAlbedo, getSampler};
        SDL_BindGPUFragmentSamplers(appState->renderPass, 0, &binding, 1);
    }

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &colorAlbedo, sizeof(glm::vec4));
}