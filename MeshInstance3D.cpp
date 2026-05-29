#include "MeshInstance3D.h"
#include "Mesh.h"
#include "Camera3D.h"
#include "Material.h"
#include <SDL3/SDL_log.h>

void MeshInstance3D::Draw(AppState &appState, SDL_GPUCommandBuffer *commandBuffer) {
    const Mesh* _mesh = appState.GetMesh(this->mesh);
    if (!_mesh || !_mesh->isOnGPU) return;

    // Bind this mesh's vertex/index buffers (same buffers for all instances)
    const SDL_GPUBufferBinding vertexBinding = { .buffer = _mesh->vertexBuffer, .offset = 0 };
    SDL_BindGPUVertexBuffers(appState.renderPass, 0, &vertexBinding, 1);
    if (!_mesh->indices.empty()) {
        const SDL_GPUBufferBinding indexBinding = { .buffer = _mesh->indexBuffer, .offset = 0 };
        SDL_BindGPUIndexBuffer(appState.renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    }

    const glm::mat4 model = globalTransform.getMatrix();
    const glm::mat4 view = appState.current_camera->GetViewMatrix();
    const glm::mat4 proj = appState.current_camera->GetProjectionMatrix(appState.currentAspectRatio);
    const glm::mat4 mvp = proj * view * model;
    SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(mvp));

    for (const auto& submesh : _mesh->submeshes) {
        const Material* material = nullptr;
        if (!appState.material_override.empty()) {
            material = appState.GetMaterial(appState.material_override);
        }
        else if (submesh.material.empty()) {
            material = appState.materials.at("missing");
        }
        else if (!appState.materials.contains(submesh.material)) {
            SDL_Log("AppState's materials does not contain %s, setting to missing...", submesh.material.c_str());
            material = appState.GetMaterial("missing");
        }
        else {
            material = appState.GetMaterial(submesh.material);
        }
        material->Bind(&appState, commandBuffer);
        if (!_mesh->indices.empty()) {
            SDL_DrawGPUIndexedPrimitives(appState.renderPass, submesh.indexCount, 1, submesh.startIndex, 0, 0);
        } else {
            SDL_DrawGPUPrimitives(appState.renderPass, submesh.vertexCount, 1, submesh.startVertex, 0);
        }
    }
}
