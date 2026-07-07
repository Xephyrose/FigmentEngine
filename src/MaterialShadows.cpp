#include "MaterialShadows.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

MaterialShadows::MaterialShadows(AppState* appState, const std::string &name, const std::string &pipeline) {
    this->name = name;
    this->pipeline = pipeline;
    appState->materials.insert_or_assign(name, this);
}

void MaterialShadows::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, glm::mat4 model) {
    if (!appState->current_camera_3d) return;
    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }
    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    glm::mat4 view = appState->current_camera_3d->GetViewMatrix();
    glm::mat4 proj = appState->current_camera_3d->GetProjectionMatrix(appState->currentAspectRatio);
    glm::mat4 mvp = proj * view * model;
    glm::mat4 lightVP = appState->GetLightViewProjection();

    struct TransformData {
        glm::mat4 mvp;
        glm::mat4 model;
        glm::mat4 lightVP;
    };
    TransformData data{};
    data.mvp = mvp;
    data.model = model;
    data.lightVP = lightVP;
    SDL_PushGPUVertexUniformData(commandBuffer, 0, &data, sizeof(data));

    SDL_GPUSampler* shadowSampler = appState->GetSampler("shadow_sampler");
    if (!shadowSampler) {
        SDL_Log("ERROR: Shadow sampler not found!");
        return;
    }

    SDL_GPUTextureSamplerBinding shadowBinding = {
        .texture = appState->shadowMap,
        .sampler = shadowSampler
    };
    SDL_BindGPUFragmentSamplers(appState->renderPass, 0, &shadowBinding, 1);
}
