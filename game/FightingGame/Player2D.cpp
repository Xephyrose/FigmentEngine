#include "Player2D.h"

#include "../../src/Input.h"
#include "../../src/Sprite2D.h"

Player2D::Player2D(AppState &appState, const float size_x, const float size_y, const float pos_x, const float pos_y) : PhysicsBody2D(appState, b2_dynamicBody, size_x, size_y, pos_x, pos_y) {
    name = "Player2D";
    auto* sprite = new Sprite2D();
    addChild(std::unique_ptr<Node>(sprite));
    b2MotionLocks locks = {};
    locks.angularZ = true;
    b2Body_SetMotionLocks(bodyId, locks);
}

void Player2D::Input(AppState &appState) {
    // Keep in mind this is the velocity at the START of the method
    b2Vec2 curVel = b2Body_GetLinearVelocity(bodyId);
    auto moveDirection = glm::vec2(0.0f);
    if (Input::IsPressed(SDL_SCANCODE_S)) {
        b2Body_SetGravityScale(bodyId, 2);
        if (curVel.y < 0) b2Body_SetLinearVelocity(bodyId, b2Vec2(curVel.x, 0));
    }
    else {
        b2Body_SetGravityScale(bodyId, 1);
    }
    if (Input::IsPressed(SDL_SCANCODE_D)) moveDirection.x += 1;
    if (Input::IsPressed(SDL_SCANCODE_A)) moveDirection.x -= 1;
    if (Input::IsJustPressed(SDL_SCANCODE_SPACE)) b2Body_ApplyLinearImpulseToCenter(bodyId, b2Vec2(0, -200), true);

    if (glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
    }

    b2Body_SetLinearVelocity(bodyId, b2Vec2(moveDirection.x * speed, b2Body_GetLinearVelocity(bodyId).y));
    PhysicsBody2D::Input(appState);
}

void Player2D::Update(AppState &appState) {
    PhysicsBody2D::Update(appState);
}
