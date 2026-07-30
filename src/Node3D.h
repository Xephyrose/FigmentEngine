#ifndef FIGMENTENGINE_NODE3D_H
#define FIGMENTENGINE_NODE3D_H
#include "Node.h"
#include "Transform3D.h"

struct Node3D : Node {
    Node3D();
    void ImGuiDraw() override;
    Transform3D localTransform;
    [[nodiscard]] virtual Transform3D GetGlobalTransform(double factor = 1.0) const;
    [[nodiscard]] virtual Transform3D GetGlobalTransformInterpolated(double factor = 1.0) const;
};


#endif //FIGMENTENGINE_NODE3D_H
