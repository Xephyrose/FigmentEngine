#ifndef FIGMENTENGINE_VERTEX_H
#define FIGMENTENGINE_VERTEX_H
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <thirdparty/glm/glm.hpp>

struct Vertex {
    glm::vec3 position = glm::vec3(0);      // x, y, z
    glm::vec2 uv = glm::vec3(0);           // u, v
    glm::vec3 normal = glm::vec3(0);      // x, y, z
    glm::vec4 tangent = glm::vec4(0);    // x, y, z, w
    explicit Vertex(float x = 0, float y = 0, float z = 0, float u = 0, float v = 0);
    Vertex(const glm::vec3 &position, const glm::vec2 &uv);
};


#endif //FIGMENTENGINE_VERTEX_H
