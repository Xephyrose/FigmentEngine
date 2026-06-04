#include "Sprite2D.h"

#include "Camera2D.h"
#include "Material.h"
#include "Mesh.h"

Sprite2D::Sprite2D() {
    name = "Sprite2D";
}

void Sprite2D::Draw(AppState& appState, SDL_GPUCommandBuffer* commandBuffer) {
    const auto quadMesh = new Mesh();
    quadMesh->CreateQuad(1, 1, 0);

    if (!quadMesh->isOnGPU) return;

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

    SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(mvp));

    Material* material = appState.GetMaterial("UnlitTextured");
    if (!material) material = appState.GetMaterial("default_sprite");
    material->Bind(&appState, commandBuffer);

    SDL_DrawGPUIndexedPrimitives(appState.renderPass, quadMesh->indices.size(), 1, 0, 0, 0);

}
