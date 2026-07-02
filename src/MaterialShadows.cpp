#include "MaterialShadows.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

MaterialShadows::MaterialShadows(AppState* appState, const std::string &name, const std::string &pipeline) {
    this->name = name;
    this->pipeline = pipeline;
    appState->materials.insert_or_assign(name, this);
}

void MaterialShadows::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, glm::mat4 model) {
    // 1. Bind the pipeline (uses "Shadows" vertex/fragment shaders)
    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }
    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    // 2. Push vertex uniforms
    glm::mat4 view = appState->current_camera_3d->GetViewMatrix();
    glm::mat4 proj = appState->current_camera_3d->GetProjectionMatrix(appState->currentAspectRatio);
    glm::mat4 mvp = proj * view * model;
    glm::mat4 lightVP = appState->GetLightViewProjection();

    struct TransformData {
        glm::mat4 mvp;        // Camera's MVP (for rendering to screen)
        glm::mat4 model;      // Model matrix (for world position)
        glm::mat4 lightVP;    // Light's VP (for shadow coordinates)
    };
    TransformData data{};
    data.mvp = mvp;
    data.model = model;
    data.lightVP = lightVP;
    SDL_PushGPUVertexUniformData(commandBuffer, 0, &data, sizeof(data));

    // 3. Bind the shadow map texture (slot 0 in the fragment shader)
    //    This is the texture that was written in the depth-only pass.
    SDL_GPUSampler* shadowSampler = appState->GetSampler("shadow_sampler");
    if (!shadowSampler) {
        SDL_Log("ERROR: Shadow sampler not found!");
        return;
    }

    SDL_GPUTextureSamplerBinding shadowBinding = {
        .texture = appState->shadowMap,
        .sampler = shadowSampler
    };
    SDL_BindGPUFragmentSamplers(appState->renderPass, 0, &shadowBinding, 1);
}
