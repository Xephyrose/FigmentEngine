#include "Camera3D.h"

#include "thirdparty/imgui/imgui.h"

Camera3D::Camera3D() {
    name = "Camera";
}

void Camera3D::ImGuiDraw() {
    Node3D::ImGuiDraw();
    ImGui::Text("Camera3D");
    ImGui::DragFloat("FOV", &fov);
    ImGui::DragFloat("Near", &nearPlane);
    ImGui::DragFloat("Far", &farPlane);
}

glm::mat4 Camera3D::GetViewMatrix() const {
    const glm::vec3 forward = GetGlobalTransform().getForward();
    const glm::vec3 target = GetGlobalTransform().position + forward;
    return glm::lookAt(GetGlobalTransform().position, target, GetGlobalTransform().getUp());
}

glm::mat4 Camera3D::GetProjectionMatrix(const float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}
