#include "Node.h"

#include <SDL3/SDL_log.h>

Node::Node(const std::string &name) : name(name) {}
Node::Node() : name("Node") {}

void Node::Update(const AppState& appState, float delta) {}

void Node::Draw(const AppState& appState) {}

void Node::Free(const AppState& appState) {}

void Node::Input(const AppState& appState, const bool* isKeyDown) {}