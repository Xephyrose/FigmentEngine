#include "Material2D.h"
#include <SDL3/SDL_log.h>

#include "Camera2D.h"

void Material2D::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, glm::mat4 model) {
    if (!appState->current_camera_2d) return;
    const glm::mat4 view = appState->current_camera_2d->GetViewMatrix();
    const glm::mat4 proj = appState->current_camera_2d->GetProjectionMatrix(static_cast<float>(appState->windowWidth), static_cast<float>(appState->windowHeight));
    const glm::mat4 mvp = proj * view * model;

    struct TransformData {
        glm::mat4 mvp;
        glm::mat4 model;
    };

    TransformData data{};
    data.mvp = mvp;
    data.model = model;

    SDL_PushGPUVertexUniformData(commandBuffer, 0, &data, sizeof(data));

    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    SDL_GPUTexture* getAlbedo = appState->GetTexture(texture);
    SDL_GPUSampler* getSampler = appState->GetSampler(sampler);

    const SDL_GPUTextureSamplerBinding binding = {getAlbedo, getSampler};
    SDL_BindGPUFragmentSamplers(appState->renderPass, 0, &binding, 1);

    struct PushData {
        glm::vec4 colorAlbedo;
        bool      useTexture;
    };
    PushData push{};
    push.colorAlbedo = this->color;
    push.useTexture = (this->texture == "none" ? 0 : 1);

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &push, sizeof(PushData));
}
