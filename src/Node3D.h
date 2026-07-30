#ifndef FIGMENTENGINE_NODE3D_H
#define FIGMENTENGINE_NODE3D_H
#include "Node.h"
#include "Transform3D.h"

struct Node3D : Node {
    Node3D();
    void ImGuiDraw() override;
    Transform3D localTransform;
    [[nodiscard]] virtual Transform3D GetGlobalTransform() const;
    [[nodiscard]] Transform3D GetGlobalTransformInterpolated(double factor = 1.0) const;
protected:
    // Need REAL methods because we can't set a default value (double factor = 1.0) on a virtual method
    [[nodiscard]] virtual Transform3D GetGlobalTransformInterpolatedREAL(double factor) const;
};


#endif //FIGMENTENGINE_NODE3D_H
