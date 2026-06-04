#include "Camera3D.h"

#include "imgui.h"

Camera3D::Camera3D() {
    name = "Camera";
}

void Camera3D::ImGuiDraw() {
    Node3D::ImGuiDraw();
    ImGui::Text("Node");
    ImGui::InputFloat("FOV", &fov);
    ImGui::InputFloat("Near Plane", &nearPlane);
    ImGui::InputFloat("Far Plane", &farPlane);
}

glm::mat4 Camera3D::GetViewMatrix() const {
    const glm::vec3 forward = localTransform.getForward();
    const glm::vec3 target = localTransform.position + forward;
    return glm::lookAt(localTransform.position, target, localTransform.getUp());
}

glm::mat4 Camera3D::GetProjectionMatrix(const float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}
