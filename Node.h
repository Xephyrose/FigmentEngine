#ifndef FIGMENTENGINE_NODE_H
#define FIGMENTENGINE_NODE_H
#include <memory>
#include <string>
#include <vector>

#include "SDL3/SDL_gpu.h"

struct AppState;

struct Node {
    virtual ~Node() = default;
    Node();
    explicit Node(std::string name);

    Node* parent = nullptr;
    std::vector<std::unique_ptr<Node>> children;

    std::string name;
    virtual void Update(const AppState& appState);
    virtual void Draw(AppState &appState, SDL_GPUCommandBuffer *commandBuffer);
    virtual void Input(const AppState& appState);
    void addChild(std::unique_ptr<Node> child);
    void killChild(Node* child);
    void killChild(const std::string& name);
};

#endif //FIGMENTENGINE_NODE_H