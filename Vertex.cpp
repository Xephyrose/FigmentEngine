#include "Vertex.h"

Vertex::Vertex(const float x, const float y, const float z, const float u, const float v) {
    position.x = x;
    position.y = y;
    position.z = z;
    uv.x = u;
    uv.y = v;
}

Vertex::Vertex(const glm::vec3 &position, const glm::vec2 &uv) : position(position), uv(uv) {}