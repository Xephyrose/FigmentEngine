#include "MaterialPhongTexturedNormalMap.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

void MaterialPhongTexturedNormalMap::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, glm::mat4 model) {
    if (!appState->current_camera_3d) return;
    const glm::mat4 view = appState->current_camera_3d->GetViewMatrix();
    const glm::mat4 proj = appState->current_camera_3d->GetProjectionMatrix(appState->currentAspectRatio);
    const glm::mat4 mvp = proj * view * model;
    const glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));

    struct TransformData {
        glm::mat4 mvp;
        glm::mat4 model;
        glm::mat4 normalMatrix;
        glm::mat4 lightVP;
    };

    TransformData data{};
    data.mvp = mvp;
    data.model = model;
    data.normalMatrix = normalMatrix;
    data.lightVP = appState->GetLightViewProjection();

    SDL_PushGPUVertexUniformData(commandBuffer, 0, &data, sizeof(data));

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
        {getAlbedo, getSampler0},              // t0
        {getAmbient, getSampler1},            // t1
        {getSpecular, getSampler2},          // t2
        {getNormalMap, getSampler3},        // t3
        {appState->shadowMap, getSampler3} // t4
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
        int       num_point_lights;
        int       num_dir_lights;
        int       num_spot_lights;
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

void MaterialPhongTexturedNormalMap::setTextureNormalMap(AppState *appState, const std::string &texture) {
    textureNormalMap = texture;
    if (textureNormalMap != "none") {
        appState->LoadTexture(textureNormalMap);
    }
}
