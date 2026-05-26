#ifndef FIGMENTENGINE_MESH_H
#define FIGMENTENGINE_MESH_H
#include <vector>

#include "AppState.h"
#include "Vertex.h"

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

    void UploadToGPU(const AppState& appState);
    void ReleaseGPUResources(const AppState* appState);

    [[nodiscard]] const Submesh *GetSubmesh(const std::string &name) const;

    static Mesh CreateQuad(float width = 1.6f, float height = 1.6f);
    static Mesh CreateTriangle(float size = 1.4f);
};

#endif //FIGMENTENGINE_MESH_H
