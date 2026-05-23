#include "MaterialUnlitTextured.h"

MaterialUnlitTextured::MaterialUnlitTextured(const std::string &name, const std::string &pipeline, const std::string& sampler, const std::string &textureAlbedo) : textureAlbedo(textureAlbedo) {
    this->name = name;
    this->pipeline = pipeline;
    this->sampler = sampler;
}

void MaterialUnlitTextured::Bind(AppState *appState) const {
    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);
    SDL_Log("SUCCESS: Pipeline '%s' bound to render pass", pipeline.c_str());

    BindTextures(appState);
}

void MaterialUnlitTextured::BindTextures(AppState *appState) const {
    SDL_GPUTexture* getAlbedo = appState->GetTexture(textureAlbedo);
    SDL_GPUSampler* getSampler = appState->GetSampler(sampler);


    if (getAlbedo && getSampler) {
        const SDL_GPUTextureSamplerBinding binding = {getAlbedo, getSampler};
        SDL_BindGPUFragmentSamplers(appState->renderPass, 0, &binding, 1);
    }
}