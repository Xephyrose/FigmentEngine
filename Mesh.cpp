#include "Mesh.h"
#include "Vertex.h"

Mesh Mesh::CreateQuad(const float width, const float height) {
        const float halfW = width * 0.5f;
        const float halfH = height * 0.5f;

        Mesh mesh;
        mesh.vertices = {
            Vertex{-halfW, -halfH, 0.0f}, // Bottom-Left
            Vertex{ halfW, -halfH, 0.0f}, // Bottom-Right
            Vertex{ halfW,  halfH, 0.0f}, // Top-Right
            Vertex{-halfW,  halfH, 0.0f}, // Top-Left
        };

        mesh.indices = {
            0, 3, 2, // Triangle 1: Bottom-Left, Top-Left, Top-Right (CCW)
            0, 2, 1  // Triangle 2: Bottom-Left, Top-Right, Bottom-Right (CCW)
        };

        return mesh;
    }

Mesh Mesh::CreateTriangle(const float size) {
        Mesh mesh;
        mesh.vertices = {
            Vertex{-size * 0.5f, -size * 0.5f, 0.0f},
            Vertex{ size * 0.5f, -size * 0.5f, 0.0f},
            Vertex{ 0.0f,        size * 0.5f, 0.0f},
        };

        mesh.indices = {0, 1, 2};

        return mesh;
    }