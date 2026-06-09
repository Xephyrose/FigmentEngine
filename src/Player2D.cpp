#include "Player2D.h"

#include "Input.h"
#include "Sprite2D.h"

Player2D::Player2D(AppState &appState, const float size_x, const float size_y, const float pos_x, const float pos_y) : PhysicsBody2D(appState, b2_dynamicBody, size_x, size_y, pos_x, pos_y) {
    name = "Player2D";
    auto* sprite = new Sprite2D();
    addChild(std::unique_ptr<Node>(sprite));
    b2MotionLocks locks = {};
    locks.angularZ = true;
    b2Body_SetMotionLocks(bodyId, locks);
}

void Player2D::Input(AppState &appState) {
    auto moveDirection = glm::vec2(0.0f);
    if (Input::IsPressed(SDL_SCANCODE_W)) moveDirection.y -= 1;
    if (Input::IsPressed(SDL_SCANCODE_S)) moveDirection.y += 1;
    if (Input::IsPressed(SDL_SCANCODE_D)) moveDirection.x += 1;
    if (Input::IsPressed(SDL_SCANCODE_A)) moveDirection.x -= 1;

    if (glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
    }

    b2Body_SetLinearVelocity(bodyId, b2Vec2(moveDirection.x, moveDirection.y) * speed);
    PhysicsBody2D::Input(appState);
}
