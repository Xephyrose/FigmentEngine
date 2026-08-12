#ifndef FIGMENTENGINE_BOXBODY3D_H
#define FIGMENTENGINE_BOXBODY3D_H
#include "PhysicsBody3D.h"

struct BoxBody3D : PhysicsBody3D {
    BoxBody3D(b3BodyType bodyType, float pos_x, float pos_y, float pos_z, float width, float height, float depth);
};

#endif //FIGMENTENGINE_BOXBODY3D_H
