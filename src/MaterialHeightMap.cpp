#include "MaterialHeightMap.h"

#include "Camera3D.h"
#include "SDL3/SDL_log.h"


void MaterialHeightMap::Bind(AppState *appState, SDL_GPUCommandBuffer* commandBuffer, const glm::mat4 model) {
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
        float HeightScale = 1;
        float TerrainSizeX = 100;
        float TerrainSizeZ = 100;
        float padding = 0;
    };

    TransformData data{};
    data.mvp = mvp;
    data.model = model;
    data.normalMatrix = normalMatrix;
    data.lightVP = appState->GetOffsetLightViewProjection();

    SDL_PushGPUVertexUniformData(commandBuffer, 0, &data, sizeof(data));

    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    SDL_GPUTexture* getAlbedo = appState->GetTexture(texture);
    // SDL_GPUTexture* getAlbedo = appState->shadowMap;
    SDL_GPUSampler* getSampler = appState->GetSampler(sampler);

    const SDL_GPUTextureSamplerBinding binding = {getAlbedo, getSampler};
    SDL_BindGPUVertexSamplers(appState->renderPass, 0, &binding, 1);

    struct PushData {
        glm::vec4 color;
    };
    PushData push{};
    push.color = color;

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &push, sizeof(PushData));
}
