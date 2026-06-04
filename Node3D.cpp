#include "Node3D.h"

#include "imgui.h"

void Node3D::ImGuiDraw() {
    Node::ImGuiDraw();
    ImGui::Text("Node3D");
    float position[3] = { localTransform.position.x, localTransform.position.y, localTransform.position.z };
    if (ImGui::InputFloat3("Position", position)) {
        localTransform.position = glm::vec3(position[0], position[1], position[2]);
    }

    float rotation[3] = { localTransform.rotation.x, localTransform.rotation.y, localTransform.rotation.z };
    if (ImGui::InputFloat3("Rotation", rotation)) {
        localTransform.setRotation(glm::vec3(rotation[0], rotation[1], rotation[2]));
    }

    float scale[3] = { localTransform.scale.x, localTransform.scale.y, localTransform.scale.z };
    if (ImGui::InputFloat3("Scale", scale)) {
        localTransform.scale = glm::vec3(scale[0], scale[1], scale[2]);
    }
}

Transform3D Node3D::GetGlobalTransform() const {
    return localTransform;
}
