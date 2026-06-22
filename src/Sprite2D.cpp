#include "Sprite2D.h"

#include "Camera2D.h"
#include "../thirdparty/imgui/imgui.h"
#include "../thirdparty/imgui/imgui_stdlib.h"
#include "Material.h"
#include "Mesh.h"

Sprite2D::Sprite2D() : size(glm::vec2(100.0f, 100.0f)) {
    name = "Sprite2D";
    sprite = "missing_2d";
}

void Sprite2D::ImGuiDraw() {
    Node2D::ImGuiDraw();
    ImGui::Text("Sprite2D");
    ImGui::InputText("Sprite", &sprite);
    float _size[2] = {size.x, size.y};
    ImGui::DragFloat2("Size", _size);
    size.x = _size[0];
    size.y = _size[1];
}

void Sprite2D::Draw(AppState& appState, SDL_GPUCommandBuffer* commandBuffer) {
    if (!appState.current_camera_2d) return;
    const Mesh* quadMesh = appState.quadMesh;

    if (!quadMesh->isOnGPU) return;

    const SDL_GPUBufferBinding vertexBinding = { .buffer = quadMesh->vertexBuffer, .offset = 0 };
    SDL_BindGPUVertexBuffers(appState.renderPass, 0, &vertexBinding, 1);
    if (!quadMesh->indices.empty()) {
        const SDL_GPUBufferBinding indexBinding = { .buffer = quadMesh->indexBuffer, .offset = 0 };
        SDL_BindGPUIndexBuffer(appState.renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    }

    Transform2D transform = GetGlobalTransform();
    transform.scale *= size;

    const glm::mat4 model = transform.getMatrix();
    const glm::mat4 view = appState.current_camera_2d->GetViewMatrix();
    const glm::mat4 proj = appState.current_camera_2d->GetProjectionMatrix(
        appState.windowWidth,
        appState.windowHeight
    );
    const glm::mat4 mvp = proj * view * model;

    SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(mvp));

    Material* material = appState.GetMaterial(sprite);
    if (material == nullptr) material = appState.GetMaterial("missing_2d");
    material->Bind(&appState, commandBuffer);

    SDL_DrawGPUIndexedPrimitives(appState.renderPass, quadMesh->indices.size(), 1, 0, 0, 0);
}
