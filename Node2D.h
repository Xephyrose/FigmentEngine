#ifndef FIGMENTENGINE_NODE2D_H
#define FIGMENTENGINE_NODE2D_H
#include "Node.h"
#include "Transform2D.h"


struct Node2D : public Node{
    Node2D();
    void ImGuiDraw() override;
    Transform2D localTransform;
    [[nodiscard]] Transform2D GetGlobalTransform() const;
};


#endif //FIGMENTENGINE_NODE2D_H
