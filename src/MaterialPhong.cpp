#include "MaterialPhong.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

MaterialPhong::MaterialPhong(AppState *appState, const std::string &name, const std::string &pipeline) {
    this->name = name;
    this->pipeline = pipeline;
    appState->materials.insert_or_assign(name, this);
}

void MaterialPhong::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, const glm::mat4 model) {
    if (!appState->current_camera_3d) return;
    BindVertexUniformDataMMNL(*appState, commandBuffer, model);

    SDL_GPUGraphicsPipeline *gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    const SDL_GPUTextureSamplerBinding bindings[] = {
        {appState->shadowMap, appState->GetSampler("anisotropic_repeat")}
    };
    SDL_BindGPUFragmentSamplers(appState->renderPass, 0, bindings, std::size(bindings));

    struct PushData {
        glm::vec4 viewPos;
        int       num_point_lights;
        int       num_dir_lights;
        int       num_spot_lights;
    };
    PushData push{};
    push.viewPos = glm::vec4(appState->current_camera_3d->GetGlobalTransformInterpolated(appState->fixedTimeStepAccumulator / appState->fixedTimeStep).position, 0);
    push.num_point_lights = appState->pointLights.size();
    push.num_dir_lights = appState->directionalLights.size();
    push.num_spot_lights = appState->spotLights.size();

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

    SDL_BindGPUFragmentStorageBuffers(
        appState->renderPass,
        2,
        &appState->spotLightBuffer,
        1
    );
}
