#include "Node.h"

#include <utility>
#include "../thirdparty/imgui/imgui.h"
#include "../thirdparty/imgui/imgui_stdlib.h"

Node::Node(std::string name) : name(std::move(name)) {}

Node::Node() : name("Node") {}

void Node::ImGuiDraw() {
    if (ImGui::CollapsingHeader("Node", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputText("Name", &name);
    }
}

void Node::Update(AppState& appState) {
    for (const auto & i : children) {
        i->Update(appState);
    }
}

void Node::Draw(AppState &appState, SDL_GPUCommandBuffer *commandBuffer) {
    for (const auto & i : children) {
        i->Draw(appState, commandBuffer);
    }
}

void Node::DrawShadow(AppState &appState, SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass* renderPass) {
    for (const auto & i : children) {
        i->DrawShadow(appState, commandBuffer, renderPass);
    }
}

void Node::Input(AppState& appState) {
    for (const auto & i : children) {
        i->Input(appState);
    }
}

void Node::Event(AppState &appState, SDL_Event &event) {
    for (const auto & i : children) {
        i->Event(appState, event);
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