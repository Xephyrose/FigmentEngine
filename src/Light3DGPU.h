#ifndef FIGMENTENGINE_POINTLIGHT3DGPU_H
#define FIGMENTENGINE_POINTLIGHT3DGPU_H
#include "../thirdparty/glm/vec4.hpp"

struct PointLight3DGPU {
    glm::vec4 color = glm::vec4(1.0f); // xyz is color, w is intensity
    glm::vec4 position = glm::vec4(0.0f); // xyz is position, w is a dummy
};

struct DirectionalLight3DGPU {
    glm::vec4 color = glm::vec4(1.0f); // xyz is color, w is intensity
    glm::vec4 direction = glm::vec4(0.0f); // xyz is direction, w is a dummy
};

struct SpotLight3DGPU {
    glm::vec4 color = glm::vec4(1.0f); // xyz is color, w is intensity
    glm::vec4 position = glm::vec4(0.0f); // xyz is position, w is a dummy
    glm::vec4 direction = glm::vec4(0.0f); // xyz is direction, w is a dummy
};

#endif //FIGMENTENGINE_POINTLIGHT3DGPU_H
