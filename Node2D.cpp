#include "Node2D.h"

#include "thirdparty/imgui/imgui.h"

Node2D::Node2D() {
    name = "Node2D";
}

void Node2D::ImGuiDraw() {
    Node::ImGuiDraw();
    ImGui::Text("Node2D");
    localTransform.ImGuiDraw();
}

Transform2D Node2D::GetGlobalTransform() const {
    if (parent != nullptr) {
        const auto* par = dynamic_cast<Node2D*> (parent);
        if (par != nullptr) {
            Transform2D globalTransform;
            globalTransform.position = par->GetGlobalTransform().position + localTransform.position;
            globalTransform.rotation = par->GetGlobalTransform().rotation + localTransform.rotation;
            globalTransform.scale = par->GetGlobalTransform().scale + localTransform.scale;
            return globalTransform;
        }
    }
    return localTransform;
}
