#include "BoxBody3D.h"

BoxBody3D::BoxBody3D(AppState &appState, b3BodyType bodyType, float pos_x, float pos_y, float pos_z, float width, float height, float depth) : PhysicsBody3D(appState, bodyType, pos_x, pos_y, pos_z) {
    b3BoxHull collider = b3MakeBoxHull(width, height, depth);

    b3ShapeDef shapeDef = b3DefaultShapeDef();
    shapeDef.baseMaterial.friction = 100.0f;
    shapeDef.density = 1.0f;

    b3CreateHullShape(bodyId, &shapeDef, &collider.base);
}
