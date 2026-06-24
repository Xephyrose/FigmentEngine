#include "MaterialPhongTextured.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

void MaterialPhongTextured::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer) {
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
    SDL_GPUSampler* getSampler0 = appState->GetSampler(sampler);
    SDL_GPUSampler* getSampler1 = appState->GetSampler(sampler);
    SDL_GPUSampler* getSampler2 = appState->GetSampler(sampler);
    SDL_GPUSampler* getSampler3 = appState->GetSampler(sampler);

    const SDL_GPUTextureSamplerBinding bindings[] = {
        {getAlbedo, getSampler0},      // t0
        {getAmbient, getSampler1},    // t1
        {getSpecular, getSampler2},  // t2
        {getNormalMap, getSampler3} // t3
    };
    SDL_BindGPUFragmentSamplers(appState->renderPass, 0, bindings, std::size(bindings));

    struct PushData {
        glm::vec3 viewPos;
        float     shininess;
        glm::vec4 colorAlbedo;
        uint32_t  useAlbedoTexture;
        glm::vec3 colorAmbient;
        uint32_t  useAmbientTexture;
        glm::vec3 colorSpecular;
        uint32_t  useSpecularTexture;
        uint32_t  useNormalMap;
    };
    PushData push{};
    push.viewPos = appState->current_camera_3d->GetGlobalTransform().position;
    push.shininess = shininess;
    push.colorAlbedo = colorAlbedo;
    push.useAlbedoTexture = (textureAlbedo == "none" ? 0 : 1);
    push.colorAmbient = glm::vec4(colorAmbient, 1.0f);
    push.useAmbientTexture = (textureAmbient == "none" ? 0 : 1);
    push.colorSpecular = glm::vec4(colorSpecular, 1.0f);
    push.useSpecularTexture = (textureSpecular == "none" ? 0 : 1);
    push.useNormalMap = (textureNormalMap == "none" ? 0 : 1);

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &push, sizeof(PushData));

    SDL_BindGPUFragmentStorageBuffers(
        appState->renderPass,
        0,
        &appState->lightBuffer,
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
