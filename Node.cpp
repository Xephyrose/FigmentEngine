#include "Node.h"

Node::Node(const std::string &name) : name(name) {}
Node::Node() : name("Node") {}

void Node::Update(const AppState& appState) {}

void Node::Draw(const AppState& appState) {}

void Node::Free(const AppState& appState) {}

void Node::Input(const AppState& appState, const bool* isKeyDown) {}