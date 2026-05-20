#include "Transform3D.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

glm::mat4 Transform3D::GetModelMatrix() const {
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = model * glm::toMat4(rotation);
    model = glm::scale(model, scale);
    return model;
}
