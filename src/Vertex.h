#ifndef FIGMENTENGINE_VERTEX_H
#define FIGMENTENGINE_VERTEX_H
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;   // x, y, z
    glm::vec2 uv;         // u, v
    explicit Vertex(float x = 0, float y = 0, float z = 0, float u = 0, float v = 0);
    Vertex(const glm::vec3 &position, const glm::vec2 &uv);
};


#endif //FIGMENTENGINE_VERTEX_H
