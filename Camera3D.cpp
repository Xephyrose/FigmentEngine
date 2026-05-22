#include "Camera3D.h"

glm::mat4 Camera3D::GetViewMatrix() const {
    const glm::vec3 forward = transform.getForward();
    const glm::vec3 target = transform.position + forward;
    return glm::lookAt(transform.position, target, transform.getUp());
}

glm::mat4 Camera3D::GetProjectionMatrix(const float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}
