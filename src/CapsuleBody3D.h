#ifndef FIGMENTENGINE_CAPSULEBODY3D_H
#define FIGMENTENGINE_CAPSULEBODY3D_H
#include "PhysicsBody3D.h"

struct CapsuleBody3D : PhysicsBody3D {
    CapsuleBody3D(AppState &appState, b3BodyType bodyType, float pos_x, float pos_y, float pos_z, float radius, float totalHeight);
    b3Capsule collider{};
};

#endif //FIGMENTENGINE_CAPSULEBODY3D_H
