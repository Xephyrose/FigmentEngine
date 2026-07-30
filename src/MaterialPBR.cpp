#include "MaterialPBR.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

void MaterialPBR::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, glm::mat4 model) {
    if (!appState->current_camera_3d) return;
    BindVertexUniformDataMMNL(*appState, commandBuffer, model);

    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    const SDL_GPUTextureSamplerBinding bindings[] = {
        {appState->shadowMap, appState->GetSampler("shadow_sampler")} // t0
    };
    SDL_BindGPUFragmentSamplers(appState->renderPass, 0, bindings, std::size(bindings));

    struct PushData {
        glm::vec4 viewPos;
        glm::vec4 colorAlbedo;
        glm::vec4 colorORM;
        glm::vec4 lightNums; // num_point_lights, num_dir_lights, num_spot_lights
    };
    PushData push{};
    push.viewPos = glm::vec4(appState->current_camera_3d->GetGlobalTransformInterpolated(appState->fixedTimeStepAccumulator).position, 0);
    push.colorAlbedo = color;
    push.colorORM.x = colorAO;
    push.colorORM.y = colorRoughness;
    push.colorORM.z = colorMetallic;
    push.lightNums.x = appState->pointLights.size();
    push.lightNums.y = appState->directionalLights.size();
    push.lightNums.z = appState->spotLights.size();

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

void MaterialPBR::setColorMetallic(const float color) {
    colorMetallic = color;
}

void MaterialPBR::setColorRoughness(const float color) {
    colorRoughness = color;
}

void MaterialPBR::setColorAO(const float color) {
    colorAO = color;
}
