#include "MaterialLit.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

MaterialLit::MaterialLit(AppState *appState, const std::string &name, const std::string &pipeline) {
    this->name = name;
    this->pipeline = pipeline;
    appState->materials.insert_or_assign(name, this);
}

void MaterialLit::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer) {
    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    SDL_BindGPUFragmentStorageBuffers(
        appState->renderPass,
        0,
        &appState->lightBuffer,
        1
    );

    struct PushData {
        glm::vec3 viewPos;
    };
    PushData push{};
    push.viewPos = appState->current_camera_3d->GetGlobalTransform().position;

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &push, sizeof(PushData));
}
