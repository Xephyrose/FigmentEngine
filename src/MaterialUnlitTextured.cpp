#include "MaterialUnlitTextured.h"
#include <SDL3/SDL_log.h>

MaterialUnlitTextured::MaterialUnlitTextured(AppState* appState, const std::string &name, const std::string &pipeline) {
    this->name = name;
    this->pipeline = pipeline;
    appState->materials.insert_or_assign(name, this);
}

void MaterialUnlitTextured::Bind(AppState *appState, SDL_GPUCommandBuffer* commandBuffer) {
    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    SDL_GPUTexture* getAlbedo = appState->GetTexture(textureAlbedo);
    SDL_GPUSampler* getSampler = appState->GetSampler(sampler);

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

void MaterialUnlitTextured::setSampler(AppState* appState, const std::string &new_sampler) {
    sampler = new_sampler;
}
