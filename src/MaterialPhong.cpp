#include "MaterialPhong.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

MaterialPhong::MaterialPhong(AppState *appState, const std::string &name, const std::string &pipeline) {
    this->name = name;
    this->pipeline = pipeline;
    appState->materials.insert_or_assign(name, this);
}

void MaterialPhong::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, const glm::mat4 model) {
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

    SDL_GPUGraphicsPipeline *gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    SDL_BindGPUFragmentStorageBuffers(
        appState->renderPass,
        0,
        &appState->pointLightBuffer,
        1
    );

    struct PushData {
        glm::vec3 viewPos;
    };
    PushData push{};
    push.viewPos = appState->current_camera_3d->GetGlobalTransform().position;

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &push, sizeof(PushData));
}
