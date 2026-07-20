#include "QuakePlayer3D.h"

#include "../../src/Input.h"
#include "src/GLMHelper.h"

void QuakePlayer3D::FixedUpdate(AppState &appState) {
    glm::vec3 velocity = GetLinearVelocity(bodyId);
    if (Input::IsJustPressed(SDL_SCANCODE_SPACE)) {
        b3Body_SetLinearVelocity(bodyId, b3Vec3(velocity.x, 0, velocity.z));
        b3Body_ApplyLinearImpulseToCenter(bodyId, b3Vec3(0, 1100, 0), true);
    }

    //CheckLanded();
    SV_AirMove(velocity, static_cast<float>(appState.fixedTimeStep));
}

void QuakePlayer3D::SV_AirMove(glm::vec3 &velocity, const float delta) const {
    const glm::vec3 forward = yaw->localTransform.getForward();
    const glm::vec3 right = yaw->localTransform.getRight();

    auto wishVel = glm::vec3(0.0f);
    if (Input::IsPressed(SDL_SCANCODE_W)) wishVel += forward * sv_maxspeed;
    if (Input::IsPressed(SDL_SCANCODE_S)) wishVel -= forward * sv_maxspeed;
    if (Input::IsPressed(SDL_SCANCODE_D)) wishVel += right * sv_maxspeed;
    if (Input::IsPressed(SDL_SCANCODE_A)) wishVel -= right * sv_maxspeed;

    float wishSpeed = glm::length(wishVel);

    const glm::vec3 wishDir = wishSpeed > 0.0f ? wishVel / wishSpeed : glm::vec3(0.0f);

    wishSpeed = std::max(wishSpeed, sv_stopspeed);

    // if is_on_floor():
    //     SV_UserFriction(delta)
    //     SV_Accelerate(wishdir, wishspeed, delta)
    // else:
    //     SV_AirAccelerate(wishdir, wishspeed, delta)

    SV_UserFriction(velocity, delta);
    SV_Accelerate(velocity, wishDir, wishSpeed, delta);

    SetLinearVelocity(bodyId, velocity);
}

void QuakePlayer3D::SV_UserFriction(glm::vec3 &velocity, const float delta) const {
    const float speed = glm::length(glm::vec3(velocity.x, 0, velocity.z));
    if (speed == 0) return;

    const float control = std::max(speed, sv_stopspeed);
    float newSpeed = speed - delta * control * sv_friction;

    newSpeed = std::max(newSpeed, 0.0f);
    newSpeed /= speed;

    velocity.x *= newSpeed;
    velocity.z *= newSpeed;
}

void QuakePlayer3D::SV_Accelerate(glm::vec3 &velocity, glm::vec3 wishDir, float wishSpeed, float delta) const {
    const float currentSpeed = glm::dot(glm::vec3(velocity.x, 0, velocity.z), wishDir);
    const float addSpeed = wishSpeed - currentSpeed;

    if (addSpeed <= 0.0f) return;

    float accelSpeed = sv_accelerate * wishSpeed * delta;
    accelSpeed = std::min(accelSpeed, addSpeed);
    velocity += accelSpeed * wishDir;
}

void QuakePlayer3D::SV_AirAccelerate(glm::vec3 &velocity, glm::vec3 wishDir, float wishSpeed, float delta) const {
    const float cappedWishSpeed = std::min(wishSpeed, 30.0f);
    const float currentSpeed = glm::dot(glm::vec3(velocity.x, 0, velocity.z), wishDir);
    const float addSpeed = cappedWishSpeed - currentSpeed;

    if (addSpeed <= 0.0f) return;

    float accelSpeed = sv_accelerate * wishSpeed * delta;
    accelSpeed = std::min(accelSpeed, addSpeed);
    velocity += accelSpeed * wishDir;
}
