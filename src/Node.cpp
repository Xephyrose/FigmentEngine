#include "Node.h"

#include <utility>

#include "AppState.h"
#include "../thirdparty/imgui/imgui.h"
#include "../thirdparty/imgui/imgui_stdlib.h"

Node::Node(std::string name) : name(std::move(name)) {}

Node::Node() : name("Node") {}

void Node::ImGuiDraw() {
    if (ImGui::CollapsingHeader("Node", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputText("Name", &name);
    }
}

void Node::Update() {
    for (const auto & i : children) {
        i->Update();
    }
}

void Node::FixedUpdate() {
    for (const auto & i : children) {
        i->FixedUpdate();
    }
}

void Node::PostPhysicsUpdate() {
    for (const auto & i : children) {
        i->PostPhysicsUpdate();
    }
}

void Node::Draw(SDL_GPUCommandBuffer *commandBuffer) {
    for (const auto & i : children) {
        i->Draw(commandBuffer);
    }
}

void Node::DrawShadow(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass* renderPass) {
    for (const auto & i : children) {
        i->DrawShadow(commandBuffer, renderPass);
    }
}

void Node::Input() {
    for (const auto & i : children) {
        i->Input();
    }
}

void Node::Event(SDL_Event &event) {
    for (const auto & i : children) {
        i->Event(event);
    }
}

void Node::addChild(std::unique_ptr<Node> child) {
    child->parent = this;
    children.push_back(std::move(child));
}

void Node::killChild(Node* child) {
    const auto it = std::ranges::find_if(children, [child](const std::unique_ptr<Node>& ptr) {
        return ptr.get() == child;
    });
    if (it != children.end()) {
        children.erase(it);
    }
}

void Node::killChild(const std::string& _name) {
    const auto it = std::ranges::find_if(children, [&_name](const std::unique_ptr<Node>& child) {
        return child->name == _name;
    });
    if (it != children.end()) {
        children.erase(it);
    }
}

Node * Node::getChild(const std::string &_name) const {
    for (const auto & i : children) {
        if (i->name == _name) {
            return i.get();
        }
    }
    return nullptr;
}
