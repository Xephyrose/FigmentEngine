#ifndef FIGMENTENGINE_NODE_H
#define FIGMENTENGINE_NODE_H
#include <string>

#include "AppState.h"

class Node {
public:
    virtual ~Node() = default;
    Node();

    explicit Node(const std::string &name);

    std::string name;
    virtual void Update(const AppState& appState, float delta);
    virtual void Draw(const AppState& appState);
    virtual void Free(const AppState& appState);
    virtual void Input(const AppState& appState, const bool* isKeyDown); // TODO: Add input handling that can be passed into Input()
};

#endif //FIGMENTENGINE_NODE_H