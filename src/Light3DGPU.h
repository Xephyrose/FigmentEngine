#ifndef FIGMENTENGINE_POINTLIGHT3DGPU_H
#define FIGMENTENGINE_POINTLIGHT3DGPU_H
#include "../thirdparty/glm/vec4.hpp"

struct PointLight3DGPU {
    glm::vec4 color = glm::vec4(1.0f); // xyz is color, w is intensity
    glm::vec4 position = glm::vec4(0.0f); // xyz is position, w is padding
    glm::vec4 params = glm::vec4(0.0f); // x is constant, y is linear, z is quadratic, w is specular influence
};

struct DirectionalLight3DGPU {
    glm::vec4 color = glm::vec4(1.0f); // xyz is color, w is intensity
    glm::vec4 direction = glm::vec4(0.0f); // xyz is direction, w is specular influence
};

struct SpotLight3DGPU {
    glm::vec4 color = glm::vec4(1.0f); // xyz is color, w is intensity
    glm::vec4 position = glm::vec4(0.0f); // xyz is position, w is cutoff
    glm::vec4 direction = glm::vec4(0.0f); // xyz is direction, w is outer cutoff
    glm::vec4 params = glm::vec4(0.0f); // x is constant, y is linear, z is quadratic, w is specular influence
};

#endif //FIGMENTENGINE_POINTLIGHT3DGPU_H
