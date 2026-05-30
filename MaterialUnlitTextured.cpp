#include "MaterialUnlitTextured.h"
#include <SDL3/SDL_log.h>

MaterialUnlitTextured::MaterialUnlitTextured(AppState* appState, const std::string &name, const std::string &pipeline) {
    this->name = name;
    this->pipeline = pipeline;
    appState->materials.insert_or_assign(name, this);
    textureAlbedo = "none";
}

void MaterialUnlitTextured::Bind(AppState *appState, SDL_GPUCommandBuffer* commandBuffer) const {
    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    if (textureAlbedo != "none") {
        SDL_GPUTexture* getAlbedo = appState->GetTexture(textureAlbedo);
        SDL_GPUSampler* getSampler = appState->GetSampler(samplerAlbedo);

        if (getAlbedo && getSampler) {
            const SDL_GPUTextureSamplerBinding binding = {getAlbedo, getSampler};
            SDL_BindGPUFragmentSamplers(appState->renderPass, 0, &binding, 1);
        }
    }

    struct PushData {
        glm::vec4 colorAlbedo;
        bool      useTexture;
    };
    PushData push{};
    push.colorAlbedo = this->colorAlbedo;
    push.useTexture = (this->textureAlbedo == "none" ? 0 : 1);

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &push, sizeof(PushData));
}

void MaterialUnlitTextured::setColorAlbedo(const glm::vec4 color) {
    colorAlbedo = color;
}

void MaterialUnlitTextured::setTextureAlbedo(AppState* appState, const std::string &texture) {
    textureAlbedo = texture;
    if (textureAlbedo != "none") {
        appState->LoadTexture(textureAlbedo);
    }
}

void MaterialUnlitTextured::setSamplerAlbedo(AppState* appState, const std::string &sampler) {
    samplerAlbedo = sampler;
}
