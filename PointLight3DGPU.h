#ifndef FIGMENTENGINE_POINTLIGHT3DGPU_H
#define FIGMENTENGINE_POINTLIGHT3DGPU_H
#include "glm/vec3.hpp"

struct PointLight3DGPU {
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    glm::vec3 position = glm::vec3(0.0f);
};

#endif //FIGMENTENGINE_POINTLIGHT3DGPU_H
