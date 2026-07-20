#include "QuakePlayer3D.h"

#include "../../src/Input.h"
#include "SDL3/SDL_log.h"
#include "src/GLMHelper.h"

void QuakePlayer3D::FixedUpdate(AppState &appState) {
    onGround = IsGrounded(appState);
    SDL_Log("Grounded: %b", onGround);
    // CheckLanded();
    // GroundNormal();
    // StepUp();
    // StickToGround();
    // Reground();

    glm::vec3 velocity = GetLinearVelocity(bodyId);
    if (Input::IsPressed(SDL_SCANCODE_SPACE) && onGround) {
        b3Body_SetLinearVelocity(bodyId, b3Vec3(velocity.x, 0, velocity.z));
        b3Body_ApplyLinearImpulseToCenter(bodyId, b3Vec3(0, 25, 0), true);
    }
    velocity = GetLinearVelocity(bodyId);

    SV_AirMove(velocity, static_cast<float>(appState.fixedTimeStep));
}

bool QuakePlayer3D::IsStandableSurface(const b3Vec3 normal) const
{
    const float maxSlopeCos = cosf( maxSlopeAngle * B3_PI / 180.0f );
    return b3Dot( normal, b3Vec3_axisY ) >= maxSlopeCos;
}

bool QuakePlayer3D::IsGrounded(const AppState &appState) const {
    auto [feetx, feety, feetz] = b3Body_GetPosition( bodyId );
    const b3Pos feet = { feetx, feety - height * 0.5f, feetz };

    const b3Pos from = { feet.x, feet.y + 0.125f, feet.z };
    const b3Pos to = { feet.x, feet.y - 0.125f, feet.z };

    float radiusScale = 1.0f;
    TraceResult tr = TraceCapsule(appState, from, to, radius, height);

    // Shrink radius if started solid or hit non-standable surface
    while ( tr.startedSolid || ( tr.hit && !IsStandableSurface( tr.normal ) ) )
    {
        radiusScale -= 0.1f;
        if ( radiusScale < 0.7f )
        {
            return false;
        }
        tr = TraceCapsule(appState, from, to, radius * radiusScale, height);
    }

    return !tr.startedSolid && tr.hit && IsStandableSurface(tr.normal);
}

void QuakePlayer3D::SV_AirMove(glm::vec3 &velocity, const float delta) const {
    const glm::vec3 forward = yaw->localTransform.getForward();
    const glm::vec3 right = yaw->localTransform.getRight();

    auto wishVel = glm::vec3(0.0f);
    if (Input::IsPressed(SDL_SCANCODE_W)) wishVel += forward * sv_maxspeed;
    if (Input::IsPressed(SDL_SCANCODE_S)) wishVel -= forward * sv_maxspeed;
    if (Input::IsPressed(SDL_SCANCODE_D)) wishVel += right * sv_maxspeed;
    if (Input::IsPressed(SDL_SCANCODE_A)) wishVel -= right * sv_maxspeed;

    const float wishSpeed = glm::length(wishVel);

    const glm::vec3 wishDir = wishSpeed > 0.0f ? wishVel / wishSpeed : glm::vec3(0.0f);

    // wishSpeed = std::max(wishSpeed, sv_stopspeed); // uhhhhh I forgor why this is here

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

void QuakePlayer3D::SV_Accelerate(glm::vec3 &velocity, const glm::vec3 &wishDir, const float wishSpeed, const float delta) const {
    const float currentSpeed = glm::dot(glm::vec3(velocity.x, 0, velocity.z), wishDir);
    const float addSpeed = wishSpeed - currentSpeed;

    if (addSpeed <= 0.0f) return;

    float accelSpeed = sv_accelerate * wishSpeed * delta;
    accelSpeed = std::min(accelSpeed, addSpeed);
    velocity += accelSpeed * wishDir;
}

void QuakePlayer3D::SV_AirAccelerate(glm::vec3 &velocity, const glm::vec3 &wishDir, const float wishSpeed, const float delta) const {
    const float cappedWishSpeed = std::min(wishSpeed, 30.0f);
    const float currentSpeed = glm::dot(glm::vec3(velocity.x, 0, velocity.z), wishDir);
    const float addSpeed = cappedWishSpeed - currentSpeed;

    if (addSpeed <= 0.0f) return;

    float accelSpeed = sv_accelerate * wishSpeed * delta;
    accelSpeed = std::min(accelSpeed, addSpeed);
    velocity += accelSpeed * wishDir;
}
