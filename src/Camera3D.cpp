#include "Camera3D.h"

#include "ImGuiWidgets.h"
#include "thirdparty/imgui/imgui.h"

Camera3D::Camera3D() {
    name = "Camera";
}

void Camera3D::ImGuiDraw() {
    Node3D::ImGuiDraw();
    if (ImGui::CollapsingHeader("Camera3D", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::ColoredDragFloat1("FOV", fov, "FOV");
        ImGui::ColoredDragFloat1("Near", nearPlane, "Near");
        ImGui::ColoredDragFloat1("Far", farPlane, "Far");
    }
}

glm::mat4 Camera3D::GetViewMatrix() const {
    const glm::vec3 forward = GetGlobalTransform().getForward();
    const glm::vec3 target = GetGlobalTransform().position + forward;
    return glm::lookAt(GetGlobalTransform().position, target, GetGlobalTransform().getUp());
}

glm::mat4 Camera3D::GetProjectionMatrix(const float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}
