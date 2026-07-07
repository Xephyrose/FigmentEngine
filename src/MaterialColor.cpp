#include "MaterialColor.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

MaterialColor::MaterialColor(AppState *appState, const std::string &name, const std::string &pipeline) {
    this->name = name;
    this->pipeline = pipeline;
    appState->materials.insert_or_assign(name, this);
}

void MaterialColor::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, glm::mat4 model) {
    if (!appState->current_camera_3d) return;
    const glm::mat4 view = appState->current_camera_3d->GetViewMatrix();
    const glm::mat4 proj = appState->current_camera_3d->GetProjectionMatrix(appState->currentAspectRatio);
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

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &color, sizeof(glm::vec4));
}

void MaterialColor::setColor(glm::vec4 col) {
    color = col;
}
