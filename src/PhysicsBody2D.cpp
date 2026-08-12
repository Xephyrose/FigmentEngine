#include "PhysicsBody2D.h"

#include "AppState.h"

PhysicsBody2D::PhysicsBody2D(b2BodyType bodyType, float size_x, float size_y, float pos_x, float pos_y) {
    name = "PhysicsBody2D";
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = bodyType;
    bodyDef.position = (b2Vec2){pos_x / 50, pos_y / 50};
    bodyId = b2CreateBody(AppState::Get().worldId2, &bodyDef);
    b2Polygon box = b2MakeBox(size_x / 50, size_y / 50);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.material.friction = 0.0f;
    shapeDef.density = 1.0f;
    b2CreatePolygonShape(bodyId, &shapeDef, &box);
}

Transform2D PhysicsBody2D::GetGlobalTransform() const {
    return localTransform;
}

void PhysicsBody2D::Update() {
    localTransform.rotation = glm::degrees(b2Rot_GetAngle(b2Body_GetRotation(bodyId)));
    localTransform.position.x = b2Body_GetPosition(bodyId).x * 50;
    localTransform.position.y = b2Body_GetPosition(bodyId).y * 50;
    // localTransform.logTransform();
    Node2D::Update();
}
