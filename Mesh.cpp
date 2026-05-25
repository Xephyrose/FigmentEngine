#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Mesh.h"
#include <tiny_gltf.h>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <string>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>

#include "Material.h"

void Mesh::UploadToGPU(const AppState& appState) {
    if (isOnGPU) return;

    // Create vertex buffer
    const SDL_GPUBufferCreateInfo vertexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = static_cast<Uint32>(vertices.size() * sizeof(Vertex))
    };
    vertexBuffer = SDL_CreateGPUBuffer(appState.device, &vertexBufferInfo);

    // Create index buffer
    const SDL_GPUBufferCreateInfo indexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = static_cast<Uint32>(indices.size() * sizeof(uint16_t))
    };
    indexBuffer = SDL_CreateGPUBuffer(appState.device, &indexBufferInfo);

    // Upload data
    SDL_GPUCommandBuffer* uploadCmd = SDL_AcquireGPUCommandBuffer(appState.device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmd);

    // Upload vertices
    SDL_GPUTransferBufferCreateInfo transferBufferInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = static_cast<Uint32>(vertices.size() * sizeof(Vertex))
    };
    SDL_GPUTransferBuffer* vertexTransfer = SDL_CreateGPUTransferBuffer(appState.device, &transferBufferInfo);
    void* vertexData = SDL_MapGPUTransferBuffer(appState.device, vertexTransfer, false);
    memcpy(vertexData, vertices.data(), vertices.size() * sizeof(Vertex));
    SDL_UnmapGPUTransferBuffer(appState.device, vertexTransfer);

    SDL_GPUTransferBufferLocation transferLocation = {
        .transfer_buffer = vertexTransfer,
        .offset = 0,
    };

    SDL_GPUBufferRegion bufferRegion = {
        .buffer = vertexBuffer,
        .offset = 0,
        .size = static_cast<Uint32>(vertices.size() * sizeof(Vertex))
    };

    SDL_UploadToGPUBuffer(copyPass, &transferLocation, &bufferRegion, false);

    transferBufferInfo.size = static_cast<Uint32>(indices.size() * sizeof(uint16_t));

    // Upload indices similarly
    SDL_GPUTransferBuffer* indexTransfer = SDL_CreateGPUTransferBuffer(appState.device, &transferBufferInfo);
    void* indexData = SDL_MapGPUTransferBuffer(appState.device, indexTransfer, false);
    memcpy(indexData, indices.data(), indices.size() * sizeof(uint16_t));
    SDL_UnmapGPUTransferBuffer(appState.device, indexTransfer);

    SDL_GPUTransferBufferLocation indexTransferLocation = {
        .transfer_buffer = indexTransfer,
        .offset = 0,
    };

    SDL_GPUBufferRegion indexBufferRegion = {
        .buffer = indexBuffer,
        .offset = 0,
        .size = static_cast<Uint32>(indices.size() * sizeof(uint16_t))  // Cast to Uint32
    };

    SDL_UploadToGPUBuffer(copyPass, &indexTransferLocation, &indexBufferRegion, false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmd);

    // Cleanup transfer buffers
    SDL_ReleaseGPUTransferBuffer(appState.device, vertexTransfer);
    SDL_ReleaseGPUTransferBuffer(appState.device, indexTransfer);

    // Set GPU buffer pointers for submeshes
    for (auto& submesh : submeshes) {
        submesh.vertexBuffer = vertexBuffer;
        submesh.indexBuffer = indexBuffer;
    }

    isOnGPU = true;
}

void Mesh::ReleaseGPUResources(const AppState* appState) {
    if (appState && appState->device) {
        if (vertexBuffer) {
            SDL_ReleaseGPUBuffer(appState->device, vertexBuffer);
            vertexBuffer = nullptr;
        }
        if (indexBuffer) {
            SDL_ReleaseGPUBuffer(appState->device, indexBuffer);
            indexBuffer = nullptr;
        }
    }
    isOnGPU = false;
}

void Mesh::DrawSubmesh(AppState* appState, const Submesh &submesh) const {
    if (!isOnGPU) return;

    // Bind vertex buffer (same for all submeshes)
    SDL_GPUBufferBinding vertexBinding = {
        .buffer = vertexBuffer,
        .offset = 0
    };
    SDL_BindGPUVertexBuffers(appState->renderPass, 0, &vertexBinding, 1);

    // Bind index buffer if indices exist
    if (!indices.empty()) {
        SDL_GPUBufferBinding indexBinding = {
            .buffer = indexBuffer,
            .offset = 0
        };
        SDL_BindGPUIndexBuffer(appState->renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);  // Fixed: _16BIT
    }

    // Apply material (sets pipeline and textures)
    if (submesh.material.data()) {
        appState->GetMaterial(submesh.material)->Bind(appState);
    }

    // Draw
    if (!indices.empty()) {
        SDL_DrawGPUIndexedPrimitives(
            appState->renderPass,
            submesh.indexCount,
            1,
            submesh.startIndex,
            submesh.startVertex,
            0
        );
    } else {
        SDL_DrawGPUPrimitives(
            appState->renderPass,
            submesh.vertexCount,
            1,
            submesh.startVertex,
            0
        );
    }
}

void Mesh::DrawAllSubmeshes(AppState* appState) const {
    if (!isOnGPU) return;

    // Sort submeshes by material to minimize state changes
    std::vector<const Submesh*> sortedSubmeshes;
    for (const auto& submesh : submeshes) {
        sortedSubmeshes.push_back(&submesh);
    }
    std::ranges::sort(sortedSubmeshes,
                      [](const Submesh* a, const Submesh* b) {
                          return a->material < b->material;
                      });

    std::string currentMaterial;

    SDL_GPUBufferBinding vertexBinding = {
        .buffer = vertexBuffer,
        .offset = 0
    };
    SDL_BindGPUVertexBuffers(appState->renderPass, 0, &vertexBinding, 1);

    if (!indices.empty()) {
        SDL_GPUBufferBinding indexBinding = {
            .buffer = indexBuffer,
            .offset = 0
        };
        SDL_BindGPUIndexBuffer(appState->renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    }

    for (const auto* submesh : sortedSubmeshes) {
        if (submesh->material != currentMaterial) {
            if (!submesh->material.empty()) {
                if (const Material* material = appState->GetMaterial(submesh->material)) {
                    material->Bind(appState);
                }
            }
            currentMaterial = submesh->material;
        }

        if (!indices.empty()) {
            SDL_DrawGPUIndexedPrimitives(
                appState->renderPass,
                static_cast<Uint32>(submesh->indexCount),
                1,
                static_cast<Uint32>(submesh->startIndex),
                static_cast<Uint32>(submesh->startVertex),
                0
            );
        } else {
            SDL_DrawGPUPrimitives(
                appState->renderPass,
                static_cast<Uint32>(submesh->vertexCount),
                1,
                static_cast<Uint32>(submesh->startVertex),
                0
            );
        }
    }
}

const Submesh *Mesh::GetSubmesh(const std::string &name) const {
    for (const auto& submesh : submeshes) {
        if (submesh.name == name) {
            return &submesh;
        }
    }
    return nullptr;
}

Mesh Mesh::CreateQuad(const float width, const float height) {
    Mesh mesh;

    const float halfW = width * 0.5f;
    const float halfH = height * 0.5f;

    mesh.vertices = {
        Vertex(glm::vec3(-halfW,  halfH, 0.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3( halfW,  halfH, 0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3( halfW, -halfH, 0.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(-halfW, -halfH, 0.0f), glm::vec2(0.0f, 1.0f))
    };

    mesh.indices = {0, 1, 2, 0, 2, 3};

    return mesh;
}

Mesh Mesh::CreateTriangle(const float size) {
    Mesh mesh;

    float halfSize = size * 0.5f;

    mesh.vertices = {
        Vertex(glm::vec3(0.0f,  halfSize, 0.0f), glm::vec2(0.5f, 0.0f)),
        Vertex(glm::vec3(-halfSize, -halfSize, 0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3( halfSize, -halfSize, 0.0f), glm::vec2(1.0f, 1.0f))
    };

    mesh.indices = {0, 1, 2};

    return mesh;
}