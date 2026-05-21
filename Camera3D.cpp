#include "Camera3D.h"

glm::mat4 Camera3D::GetViewMatrix() const {
    return glm::lookAt(transform.position, ZERO, UP);
}

glm::mat4 Camera3D::GetProjectionMatrix(const float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}
