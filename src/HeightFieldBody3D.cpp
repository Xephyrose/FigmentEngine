#include "HeightFieldBody3D.h"

HeightFieldBody3D::HeightFieldBody3D(AppState &appState, b3BodyType bodyType, float pos_x, float pos_y, float pos_z, float *heights, int countX, int countZ, b3Vec3 scale) : PhysicsBody3D(appState, bodyType, pos_x, pos_y, pos_z) {
    b3HeightFieldDef collider{heights, nullptr, scale, countX, countZ, 0, 100, false};

    b3ShapeDef shapeDef = b3DefaultShapeDef();
    shapeDef.baseMaterial.friction = 0.0f;
    shapeDef.density = 1.0f;

    b3HeightFieldData* heightFieldData = b3CreateHeightField(&collider);

    shapeId = b3CreateHeightFieldShape(bodyId, &shapeDef, heightFieldData);
}

HeightFieldBody3D::HeightFieldBody3D(AppState &appState, b3BodyType bodyType, float pos_x, float pos_y, float pos_z, b3HeightFieldDef collider) : PhysicsBody3D(appState, bodyType, pos_x, pos_y, pos_z) {
    b3ShapeDef shapeDef = b3DefaultShapeDef();
    shapeDef.baseMaterial.friction = 0.0f;
    shapeDef.density = 1.0f;

    b3HeightFieldData* heightFieldData = b3CreateHeightField(&collider);

    shapeId = b3CreateHeightFieldShape(bodyId, &shapeDef, heightFieldData);
}
