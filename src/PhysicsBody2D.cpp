#include "PhysicsBody2D.h"

#include "AppState.h"
#include "SDL3/SDL_log.h"

PhysicsBody2D::PhysicsBody2D(AppState &appState, b2BodyType bodyType, float size_x, float size_y, float pos_x, float pos_y) {
    name = "PhysicsBody2D";
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = bodyType;
    bodyDef.position = (b2Vec2){pos_x, pos_y};
    bodyId = b2CreateBody(appState.worldId, &bodyDef);
    b2Polygon box = b2MakeBox(size_x, size_y);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(bodyId, &shapeDef, &box);
}

Transform2D PhysicsBody2D::GetGlobalTransform() const {
    return localTransform;
}

void PhysicsBody2D::Update(AppState &appState) {
    localTransform.rotation = glm::degrees(b2Rot_GetAngle(b2Body_GetRotation(bodyId)));
    localTransform.position.x = b2Body_GetPosition(bodyId).x;
    localTransform.position.y = b2Body_GetPosition(bodyId).y;
    // localTransform.logTransform();
    Node2D::Update(appState);
}
