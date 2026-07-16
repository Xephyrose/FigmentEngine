#include "MaterialPBR.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

void MaterialPBR::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, glm::mat4 model) {
    if (!appState->current_camera_3d) return;
    BindVertexUniformDataMMNL(appState, commandBuffer, model);

    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    // SDL_GPUTexture* getAlbedo = appState->GetTexture(textureAlbedo);
    // SDL_GPUTexture* getMetallic = appState->GetTexture(textureMetallic);
    // SDL_GPUTexture* getRoughness = appState->GetTexture(textureRoughness);
    // SDL_GPUTexture* getAO = appState->GetTexture(textureAO);
    // SDL_GPUSampler* getSampler0 = appState->GetSampler(sampler);
    // SDL_GPUSampler* getSampler1 = appState->GetSampler(sampler);
    // SDL_GPUSampler* getSampler2 = appState->GetSampler(sampler);
    // SDL_GPUSampler* getSampler3 = appState->GetSampler(sampler);
    // SDL_GPUSampler* getSampler4 = appState->GetSampler(sampler);

    // const SDL_GPUTextureSamplerBinding bindings[] = {
        // {getAlbedo, getSampler0},       // t0
        // {getMetallic, getSampler1},    // t1
        // {getRoughness, getSampler2},  // t2
        // {getAO, getSampler3},        // t3
        // {appState->shadowMap, getSampler4} // t4
    // };
    // SDL_BindGPUFragmentSamplers(appState->renderPass, 0, bindings, std::size(bindings));

    struct PushData {
        glm::vec4 viewPos;
        glm::vec4 colorAlbedo;
        // uint32_t  useAlbedoTexture;
        float     colorMetallic;
        // uint32_t  useMetallicTexture;
        float     colorRoughness;
        // uint32_t  useRoughnessTexture;
        float     colorAO;
        // uint32_t  useAOTexture;
        int       num_point_lights;
        int       num_dir_lights;
        int       num_spot_lights;
    };
    PushData push{};
    push.viewPos = glm::vec4(appState->current_camera_3d->GetGlobalTransform().position, 0);
    push.colorAlbedo = colorAlbedo;
    // push.useAlbedoTexture = (textureAlbedo == "none" ? 0 : 1);
    push.colorMetallic = colorMetallic;
    // push.useMetallicTexture = (textureMetallic == "none" ? 0 : 1);
    push.colorRoughness = colorRoughness;
    // push.useRoughnessTexture = (textureRoughness == "none" ? 0 : 1);
    push.colorAO = colorAO;
    // push.useAOTexture = (textureAO == "none" ? 0 : 1);
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

void MaterialPBR::setColorMetallic(const float color) {
    colorMetallic = color;
}

void MaterialPBR::setColorRoughness(const float color) {
    colorRoughness = color;
}

void MaterialPBR::setColorAO(const float color) {
    colorAO = color;
}
