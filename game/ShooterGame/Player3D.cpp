#include "Player3D.h"

#include "../../src/Input.h"
#include "SDL3/SDL_log.h"

Player3D::Player3D(AppState &appState, const float pos_x, const float pos_y, const float pos_z, float radius, float height) : CapsuleBody3D(appState, b3_dynamicBody, pos_x, pos_y, pos_z, radius, height) {
    name = "Player3D";
    b3MotionLocks locks = {};
    locks.angularX = true;
    locks.angularY = true;
    locks.angularZ = true;
    b3Body_SetMotionLocks(bodyId, locks);
}

void Player3D::FixedUpdate(AppState &appState) {
    // Keep in mind this is the velocity at the START of the method
    auto [velX, velY, velZ] = b3Body_GetLinearVelocity(bodyId);
    auto moveDirection = glm::vec3(0.0f);
    if (Input::IsPressed(SDL_SCANCODE_LCTRL)) {
        b3Body_SetGravityScale(bodyId, 2);
        if (velY < 0) b3Body_SetLinearVelocity(bodyId, b3Vec3(velX, 0, velZ));
    }
    else {
        b3Body_SetGravityScale(bodyId, 1);
    }
    if (Input::IsPressed(SDL_SCANCODE_D)) moveDirection.x += 1;
    if (Input::IsPressed(SDL_SCANCODE_A)) moveDirection.x -= 1;
    if (Input::IsPressed(SDL_SCANCODE_S)) moveDirection.z += 1;
    if (Input::IsPressed(SDL_SCANCODE_W)) moveDirection.z -= 1;
    if (Input::IsJustPressed(SDL_SCANCODE_SPACE)) b3Body_ApplyLinearImpulseToCenter(bodyId, b3Vec3(0, 2, 0), true);

    if (glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
    }

    auto p = b3Body_GetPosition(bodyId);
    SDL_Log("Position: %f %f %f", p.x, p.y, p.z);
    b3Body_SetLinearVelocity(bodyId, b3Vec3(moveDirection.x * speed, b3Body_GetLinearVelocity(bodyId).y, moveDirection.z * speed));
}
