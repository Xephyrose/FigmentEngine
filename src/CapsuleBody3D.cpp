#include "CapsuleBody3D.h"

CapsuleBody3D::CapsuleBody3D(const b3BodyType bodyType, const float pos_x, const float pos_y, const float pos_z, float radius, float totalHeight) : PhysicsBody3D(bodyType, pos_x, pos_y, pos_z) {
    collider.radius = radius;
    collider.center1 = b3Vec3(0, totalHeight * 0.5f - radius, 0);
    collider.center2 = b3Vec3(0, -totalHeight * 0.5f + radius, 0);

    b3ShapeDef shapeDef = b3DefaultShapeDef();
    shapeDef.baseMaterial.friction = 0.0f;
    shapeDef.density = 1.0f;

    shapeId = b3CreateCapsuleShape(bodyId, &shapeDef, &collider);
}
