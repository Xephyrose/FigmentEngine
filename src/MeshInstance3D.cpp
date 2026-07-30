#include "MeshInstance3D.h"
#include "Mesh.h"
#include "Camera3D.h"
#include "Material.h"
#include "SDL3/SDL_log.h"

MeshInstance3D::MeshInstance3D() {
    name = "MeshInstance3D";
}

void MeshInstance3D::Draw(AppState &appState, SDL_GPUCommandBuffer *commandBuffer) {
    const Mesh* _mesh = appState.GetMesh(this->mesh);
    if (!_mesh || !_mesh->isOnGPU) return;

    // Bind this editorMesh's vertex/index buffers (same buffers for all instances)
    const SDL_GPUBufferBinding vertexBinding = { .buffer = _mesh->vertexBuffer, .offset = 0 };
    SDL_BindGPUVertexBuffers(appState.renderPass, 0, &vertexBinding, 1);
    if (!_mesh->indices.empty()) {
        const SDL_GPUBufferBinding indexBinding = { .buffer = _mesh->indexBuffer, .offset = 0 };
        SDL_BindGPUIndexBuffer(appState.renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    }

    for (const auto& submesh : _mesh->submeshes) {
        Material* material = nullptr;
        if (!appState.material_override.empty()) {
            material = appState.GetMaterial(appState.material_override);
        }
        else if (submesh.material.empty()) {
            material = appState.materials.at("missing");
        }
        else if (!appState.materials.contains(submesh.material)) {
            // SDL_Log("AppState's materials does not contain %s, setting to missing...", submesh.material.c_str());
            material = appState.GetMaterial("missing");
        }
        else {
            material = appState.GetMaterial(submesh.material);
        }
        material->Bind(&appState, commandBuffer, GetGlobalTransformInterpolated(appState.fixedTimeStepAccumulator / appState.fixedTimeStep).getMatrix());
        if (!_mesh->indices.empty()) {
            SDL_DrawGPUIndexedPrimitives(appState.renderPass, submesh.indexCount, 1, submesh.startIndex, 0, 0);
        } else {
            SDL_DrawGPUPrimitives(appState.renderPass, submesh.vertexCount, 1, submesh.startVertex, 0);
        }
    }
    for (const auto & i : children) {
        i->Draw(appState, commandBuffer);
    }
}

void MeshInstance3D::DrawShadow(AppState &appState, SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass* renderPass) {
    const Mesh* _mesh = appState.GetMesh(this->mesh);
    if (!_mesh || !_mesh->isOnGPU) return;

    const SDL_GPUBufferBinding vertexBinding = { .buffer = _mesh->vertexBuffer, .offset = 0 };
    SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);
    if (!_mesh->indices.empty()) {
        const SDL_GPUBufferBinding indexBinding = { .buffer = _mesh->indexBuffer, .offset = 0 };
        SDL_BindGPUIndexBuffer(renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    }

    for (const auto& submesh : _mesh->submeshes) {

        // We could make a class and call its bind() here, but instead I decided to just write the bind code right here. I'm gonna regret this.
        SDL_GPUGraphicsPipeline* shadowPipeline = appState.shadowPipeline;
        if (!shadowPipeline) {
            SDL_Log("ERROR: ShadowPipeline not found!");
            return;
        }

        // 2. Bind the shadow pipeline
        SDL_BindGPUGraphicsPipeline(renderPass, shadowPipeline);

        // 3. Compute LightVP × Model
        glm::mat4 lightVP = appState.GetOffsetLightViewProjection();
        glm::mat4 model = GetGlobalTransformInterpolated(appState.fixedTimeStepAccumulator / appState.fixedTimeStep).getMatrix();
        glm::mat4 mvp = lightVP * model;

        // 4. Push to vertex shader
        SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvp, sizeof(mvp));

        if (!_mesh->indices.empty()) {
            SDL_DrawGPUIndexedPrimitives(renderPass, submesh.indexCount, 1, submesh.startIndex, 0, 0);
        } else {
            SDL_DrawGPUPrimitives(renderPass, submesh.vertexCount, 1, submesh.startVertex, 0);
        }
    }
    for (const auto & i : children) {
        i->DrawShadow(appState, commandBuffer, renderPass);
    }
}
