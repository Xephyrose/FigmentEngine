#include "MaterialPhongTextured.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

void MaterialPhongTextured::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, const glm::mat4 model) {
    if (!appState->current_camera_3d) return;
    BindVertexUniformDataMMNL(appState, commandBuffer, model);

    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    SDL_GPUTexture* getAlbedo = appState->GetTexture(textureAlbedo);
    SDL_GPUTexture* getAmbient = appState->GetTexture(textureAmbient);
    SDL_GPUTexture* getSpecular = appState->GetTexture(textureSpecular);
    SDL_GPUTexture* getNormalMap = appState->GetTexture(textureNormalMap);
    SDL_GPUSampler* getSamplerAlbedo = appState->GetSampler(sampler);
    SDL_GPUSampler* getSamplerAmbient = appState->GetSampler(sampler);
    SDL_GPUSampler* getSamplerSpecular = appState->GetSampler(sampler);
    SDL_GPUSampler* getSamplerNormalMap = appState->GetSampler(sampler);
    SDL_GPUSampler* getSamplerShadowMap = appState->GetSampler("shadow_sampler");

    const SDL_GPUTextureSamplerBinding bindings[] = {
        {getAlbedo, getSamplerAlbedo},                 // t0
        {getAmbient, getSamplerAmbient},              // t1
        {getSpecular, getSamplerSpecular},           // t2
        {getNormalMap, getSamplerNormalMap},        // t3
        {appState->shadowMap, getSamplerShadowMap} // t4
    };
    SDL_BindGPUFragmentSamplers(appState->renderPass, 0, bindings, std::size(bindings));

    struct PushData {
        glm::vec4 viewPos;
        glm::vec4 colorAlbedo;
        glm::uvec4 texturesUsed; // albedo, ambient, specular, normal
        glm::vec4 colorAmbient;
        glm::vec4 colorSpecular;
        glm::vec4 lightNums; // num_point_lights, num_dir_lights, num_spot_lights
        glm::vec4 params; // shininess
    };
    PushData push{};
    push.viewPos = glm::vec4(appState->current_camera_3d->GetGlobalTransform().position, 0);
    push.params.x = shininess;
    push.colorAlbedo = colorAlbedo;
    push.texturesUsed.x = textureAlbedo == "none" ? 0 : 1;
    push.colorAmbient = glm::vec4(colorAmbient, 0.0f);
    push.texturesUsed.y = textureAmbient == "none" ? 0 : 1;
    push.colorSpecular = glm::vec4(colorSpecular, 0.0f);
    push.texturesUsed.z = textureSpecular == "none" ? 0 : 1;
    push.texturesUsed.w = textureNormalMap == "none" ? 0 : 1;
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

void MaterialPhongTextured::setColorAmbient(const glm::vec4 color) {
    colorAmbient = color;
}

void MaterialPhongTextured::setTextureAmbient(AppState *appState, const std::string &texture) {
    textureAmbient = texture;
    if (textureAmbient != "none") {
        appState->LoadTexture(textureAmbient);
    }
}

void MaterialPhongTextured::setColorSpecular(const glm::vec4 color) {
    colorSpecular = color;
}

void MaterialPhongTextured::setTextureSpecular(AppState *appState, const std::string &texture) {
    textureSpecular = texture;
    if (textureSpecular != "none") {
        appState->LoadTexture(textureSpecular);
    }
}

void MaterialPhongTextured::setTextureNormalMap(AppState *appState, const std::string &texture) {
    textureNormalMap = texture;
    if (textureNormalMap != "none") {
        appState->LoadTexture(textureNormalMap);
    }
}
