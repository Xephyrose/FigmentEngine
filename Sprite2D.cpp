#include "Sprite2D.h"

#include "Camera2D.h"
#include "Material.h"
#include "Mesh.h"
#include "SDL3/SDL_log.h"

Sprite2D::Sprite2D() {
    name = "Sprite2D";
    sprite = "missing_2d";
    localTransform.position = glm::vec2(100.0f, 100.0f);
}

void Sprite2D::Draw(AppState& appState, SDL_GPUCommandBuffer* commandBuffer) {
    const Mesh* quadMesh = appState.quadMesh;

    if (!quadMesh->isOnGPU) return;
    SDL_Log("Testing draw now!");

    const SDL_GPUBufferBinding vertexBinding = { .buffer = quadMesh->vertexBuffer, .offset = 0 };
    SDL_BindGPUVertexBuffers(appState.renderPass, 0, &vertexBinding, 1);
    if (!quadMesh->indices.empty()) {
        const SDL_GPUBufferBinding indexBinding = { .buffer = quadMesh->indexBuffer, .offset = 0 };
        SDL_BindGPUIndexBuffer(appState.renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    }

    const glm::mat4 model = GetGlobalTransform().getMatrix();
    const glm::mat4 view = appState.current_camera_2d->GetViewMatrix();
    const glm::mat4 proj = appState.current_camera_2d->GetProjectionMatrix(
        appState.windowWidth,
        appState.windowHeight
    );
    const glm::mat4 mvp = proj * view * model;
    glm::vec4 ndc = mvp * glm::vec4(0,0,0,1); // center of quad
    SDL_Log("Sprite center NDC: (%f, %f, %f)", ndc.x, ndc.y, ndc.z);
    SDL_Log("Proj matrix: [%f %f %f %f]", proj[0][0], proj[0][1], proj[0][2], proj[0][3]);
    SDL_Log("View matrix: [%f %f %f %f]", view[0][0], view[0][1], view[0][2], view[0][3]);
    SDL_Log("Model matrix position: (%f, %f, %f)", model[3][0], model[3][1], model[3][2]);

    SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(mvp));

    Material* material = appState.GetMaterial(sprite);
    if (!material) material = appState.GetMaterial("default_sprite");
    material->Bind(&appState, commandBuffer);

    SDL_DrawGPUIndexedPrimitives(appState.renderPass, quadMesh->indices.size(), 1, 0, 0, 0);
    SDL_Log("Done draw!");
}
