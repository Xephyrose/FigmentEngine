#include "Camera2D.h"

#include "../thirdparty/imgui/imgui.h"

Camera2D::Camera2D() {
    name = "Camera2D";
}

void Camera2D::ImGuiDraw() {
    Node2D::ImGuiDraw();
    if (ImGui::CollapsingHeader("Camera2D", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat("Zoom", &zoom);
    }
}

glm::mat4 Camera2D::GetViewMatrix() const {
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-GetGlobalTransform().position, 0.0f));
    view = glm::rotate(view, glm::radians(GetGlobalTransform().rotation), glm::vec3(0, 0, 1));
    view = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));
    return view;
}

glm::mat4 Camera2D::GetProjectionMatrix(const float screenWidth, const float screenHeight) const {
    return glm::ortho(0.0f, screenWidth, screenHeight, 0.0f, -1.0f, 1.0f);
}
