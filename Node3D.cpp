#include "Node3D.h"

#include "imgui.h"

Node3D::Node3D() {
    name = "Node3D";
}

void Node3D::ImGuiDraw() {
    Node::ImGuiDraw();
    ImGui::Text("Node3D");
    localTransform.ImGuiDraw();
}

Transform3D Node3D::GetGlobalTransform() const {
    return localTransform;
}
