#include "MaterialUnlitTextured.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

MaterialUnlitTextured::MaterialUnlitTextured(AppState* appState, const std::string &name, const std::string &pipeline) {
    this->name = name;
    this->pipeline = pipeline;
    appState->materials.insert_or_assign(name, this);
}

void MaterialUnlitTextured::Bind(AppState *appState, SDL_GPUCommandBuffer* commandBuffer, const glm::mat4 model) {
    const glm::mat4 view = appState->current_camera_3d->GetViewMatrix();
    const glm::mat4 proj = appState->current_camera_3d->GetProjectionMatrix(appState->currentAspectRatio);
    const glm::mat4 mvp = proj * view * model;
    // const glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));

    struct TransformData {
        glm::mat4 mvp;
        glm::mat4 model;
        // glm::mat4 normalMatrix;
    };

    TransformData data{};
    data.mvp = mvp;
    data.model = model;
    // data.normalMatrix = normalMatrix;

    SDL_PushGPUVertexUniformData(commandBuffer, 0, &data, sizeof(data));

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
