#ifndef FIGMENTENGINE_NODE3D_H
#define FIGMENTENGINE_NODE3D_H
#include "Node.h"
#include "Transform3D.h"

struct Node3D : public Node {
    Transform3D localTransform;
    Transform3D globalTransform;
};


#endif //FIGMENTENGINE_NODE3D_H
