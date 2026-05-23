#ifndef FIGMENTENGINE_MESH_H
#define FIGMENTENGINE_MESH_H
#include <vector>

#include "AppState.h"
#include "Vertex.h"
#include "Material.h"

struct Submesh {
    std::string name;
    std::string meshName;
    SDL_GPUGraphicsPipeline* pipeline = nullptr;
    size_t startVertex;
    size_t vertexCount;
    size_t startIndex;
    size_t indexCount;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<Submesh> submeshes;

    [[nodiscard]] const Submesh *GetSubmesh(const std::string &name) const;

    static Mesh CreateQuad(float width = 1.6f, float height = 1.6f);
    static Mesh CreateTriangle(float size = 1.4f);

    static Mesh LoadGLB(const AppState& appState, std::string filepath);
};

#endif //FIGMENTENGINE_MESH_H
