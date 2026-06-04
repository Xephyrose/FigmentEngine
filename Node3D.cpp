#include "Node3D.h"

#include "imgui.h"

void Node3D::ImGuiDraw() {
    Node::ImGuiDraw();
    ImGui::Text("Node3D");
    localTransform.ImGuiDraw();
}

Transform3D Node3D::GetGlobalTransform() const {
    return localTransform;
}
