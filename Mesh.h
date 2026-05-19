#ifndef FIGMENTENGINE_MESH_H
#define FIGMENTENGINE_MESH_H
#include <cstdint>
#include <vector>
#include "Vertex.h"

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    static Mesh CreateQuad(const float width = 1.6f, const float height = 1.6f);
    static Mesh CreateTriangle(const float size = 1.4f);

    static Mesh LoadGLB(const std::string& filepath);
    static Mesh LoadGLBFromMemory(const std::vector<uint8_t>& data);
};

#endif //FIGMENTENGINE_MESH_H
