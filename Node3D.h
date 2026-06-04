#ifndef FIGMENTENGINE_NODE3D_H
#define FIGMENTENGINE_NODE3D_H
#include "Node.h"
#include "Transform3D.h"

struct Node3D : public Node {
    Transform3D localTransform;
    [[nodiscard]] Transform3D GetGlobalTransform() const;
};


#endif //FIGMENTENGINE_NODE3D_H
