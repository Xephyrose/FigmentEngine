#ifndef FIGMENTENGINE_NODE_H
#define FIGMENTENGINE_NODE_H
#include <memory>
#include <string>
#include <vector>

#include "Resource.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_gpu.h"

struct AppState;

struct Node : Resource {
    Node();
    explicit Node(std::string name);

    Node* parent = nullptr;
    std::vector<std::unique_ptr<Node>> children;

    std::string name;
    void ImGuiDraw() override;
    virtual void Update();
    virtual void FixedUpdate();
    virtual void PostPhysicsUpdate();
    virtual void Draw(SDL_GPUCommandBuffer *commandBuffer);
    virtual void DrawShadow(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass);
    virtual void Input();
    virtual void Event(SDL_Event &event);
    void addChild(std::unique_ptr<Node> child);
    void killChild(Node* child);
    void killChild(const std::string& _name);
    [[nodiscard]] Node* getChild(const std::string& _name) const;
};

#endif //FIGMENTENGINE_NODE_H