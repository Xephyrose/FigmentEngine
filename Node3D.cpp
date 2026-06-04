#include "Node3D.h"

Transform3D Node3D::GetGlobalTransform() const {
    return localTransform;
}
