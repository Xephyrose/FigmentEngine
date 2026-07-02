#include "MaterialShadows.h"
#include <SDL3/SDL_log.h>

MaterialShadows::MaterialShadows(AppState* appState, const std::string &name, const std::string &pipeline) {
    this->name = name;
    this->pipeline = pipeline;
    appState->materials.insert_or_assign(name, this);
}

void MaterialShadows::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, glm::mat4 model) {
    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }
    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    const glm::mat4 lightViewProj = appState->GetLightViewProjection();
    const glm::mat4 mvp = lightViewProj * model;

    SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(mvp));
}
