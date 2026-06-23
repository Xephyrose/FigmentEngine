#include "MaterialLitTextured.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

void MaterialLitTextured::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer) {
    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    SDL_GPUTexture* getAlbedo = appState->GetTexture(textureAlbedo);
    SDL_GPUTexture* getAmbient = appState->GetTexture(textureAmbient);
    SDL_GPUTexture* getSpecular = appState->GetTexture(textureSpecular);
    SDL_GPUSampler* getSampler0 = appState->GetSampler(sampler);
    SDL_GPUSampler* getSampler1 = appState->GetSampler(sampler);
    SDL_GPUSampler* getSampler2 = appState->GetSampler(sampler);

    const SDL_GPUTextureSamplerBinding bindings[] = {
        {getAlbedo, getSampler0},    // t0
        {getAmbient, getSampler1},  // t1
        {getSpecular, getSampler2} // t2
    };
    SDL_BindGPUFragmentSamplers(appState->renderPass, 0, bindings, std::size(bindings));

    struct PushData {
        glm::vec3 viewPos;
        float     shininess;
        glm::vec4 colorAlbedo;
        bool      useAlbedoTexture;
        glm::vec3 colorAmbient;
        bool      useAmbientTexture;
        glm::vec3 colorSpecular;
        bool      useSpecularTexture;
    };
    PushData push{};
    push.viewPos = appState->current_camera_3d->GetGlobalTransform().position;
    push.shininess = shininess;
    push.colorAlbedo = colorAlbedo;
    push.useAlbedoTexture = (textureAlbedo == "none" ? 0 : 1);
    push.colorAmbient = colorAmbient;
    push.useAmbientTexture = (textureAmbient == "none" ? 0 : 1);
    push.colorSpecular = colorSpecular;
    push.useSpecularTexture = (textureSpecular == "none" ? 0 : 1);

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &push, sizeof(PushData));

    SDL_BindGPUFragmentStorageBuffers(
        appState->renderPass,
        0,
        &appState->lightBuffer,
        1
    );
}
