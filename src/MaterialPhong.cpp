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
    };

    TransformData data{};
    data.mvp = mvp;
    data.model = model;

    SDL_PushGPUVertexUniformData(commandBuffer, 0, &data, sizeof(data));

    SDL_GPUGraphicsPipeline *gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    struct PushData {
        glm::vec3 viewPos;
        int       num_point_lights;
        int       num_dir_lights;
        // int       num_spot_lights;
    };
    PushData push{};
    push.viewPos = appState->current_camera_3d->GetGlobalTransform().position;
    push.num_point_lights = appState->pointLights.size();
    push.num_dir_lights = appState->directionalLights.size();

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &push, sizeof(PushData));

    SDL_BindGPUFragmentStorageBuffers(
        appState->renderPass,
        0,
        &appState->pointLightBuffer,
        1
    );

    SDL_BindGPUFragmentStorageBuffers(
        appState->renderPass,
        1,
        &appState->directionalLightBuffer,
        1
    );
}
