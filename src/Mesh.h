#ifndef FIGMENTENGINE_MESH_H
#define FIGMENTENGINE_MESH_H
#include <vector>

#include "Vertex.h"
#include <SDL3/SDL_gpu.h>

struct Submesh {
    std::string name;
    std::string meshName;
    std::string material;
    size_t startVertex;
    size_t vertexCount;
    size_t startIndex;
    size_t indexCount;

    SDL_GPUBuffer* vertexBuffer = nullptr;
    SDL_GPUBuffer* indexBuffer = nullptr;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<Submesh> submeshes;

    SDL_GPUBuffer* vertexBuffer = nullptr;
    SDL_GPUBuffer* indexBuffer = nullptr;
    bool isOnGPU = false;

    void UploadToGPU();
    void ReleaseGPUResources();

    [[nodiscard]] const Submesh *GetSubmesh(const std::string &name) const;

    void CreateQuad(float width = 1.0f, float height = 1.0f, float depth = 0);

    void CreateTriangle(float size = static_cast<float>(sqrt(2)), float depth = 0);
};

#endif //FIGMENTENGINE_MESH_H
