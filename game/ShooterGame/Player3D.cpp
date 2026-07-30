#include "Player3D.h"

#include "../../src/Input.h"
#include "SDL3/SDL_log.h"
#include "src/AppState.h"
#include "src/Camera3D.h"
#include "src/GLMHelper.h"

#include "src/MeshInstance3D.h"

Player3D::Player3D(AppState &appState, const float pos_x, const float pos_y, const float pos_z, const float radius, const float height) : CapsuleBody3D(appState, b3_dynamicBody, pos_x, pos_y, pos_z, radius, height), height(height), radius(radius) {
    name = "Player3D";
    b3MotionLocks locks = {};
    locks.angularX = true;
    locks.angularY = true;
    locks.angularZ = true;
    b3Body_SetMotionLocks(bodyId, locks);

    yaw = new Node3D();
    yaw->name = "Yaw";
    yaw->localTransform.position = glm::vec3(0.0f, height * 0.5f - radius, 0.0f);
    addChild(std::unique_ptr<Node>(yaw));

    pitch = new Node3D();
    pitch->name = "Pitch";
    yaw->addChild(std::unique_ptr<Node>(pitch));

    cam = new Camera3D();
    appState.current_camera_3d = cam;
    pitch->addChild(std::unique_ptr<Node>(cam));

    auto* meshInstance2 = new MeshInstance3D();
    meshInstance2->mesh = "lynx.glb";
    meshInstance2->localTransform.position = glm::vec3(0.35f, -0.5f, -0.25f);
    meshInstance2->localTransform.setRotation(glm::vec3(0, 180, 0));
    pitch->addChild(std::unique_ptr<Node>(meshInstance2));
}

void Player3D::FixedUpdate(AppState &appState) {
    IsGrounded(appState);
    b3Body_SetGravityScale(bodyId, !onGround);

    glm::vec3 velocity = GetLinearVelocity(bodyId);
    if ((Input::IsJustPressed(SDL_SCANCODE_SPACE) or (autobhop && Input::IsPressed(SDL_SCANCODE_SPACE))) && onGround) {
        b3Body_SetGravityScale(bodyId, true);
        b3Body_SetLinearVelocity(bodyId, b3Vec3(velocity.x, 0, velocity.z));
        b3Body_ApplyLinearImpulseToCenter(bodyId, b3Vec3(0, 9, 0), true);
    }

    const glm::vec3 forward = yaw->localTransform.getForward();
    const glm::vec3 right = yaw->localTransform.getRight();

    auto moveDirection = glm::vec3(0.0f);
    if (Input::IsPressed(SDL_SCANCODE_W)) moveDirection += forward;
    if (Input::IsPressed(SDL_SCANCODE_S)) moveDirection -= forward;
    if (Input::IsPressed(SDL_SCANCODE_D)) moveDirection += right;
    if (Input::IsPressed(SDL_SCANCODE_A)) moveDirection -= right;
    if (Input::IsPressed(SDL_SCANCODE_SPACE)) moveDirection += UP;
    if (Input::IsPressed(SDL_SCANCODE_LCTRL)) moveDirection -= UP;

    if (Input::IsPressed(SDL_SCANCODE_LCTRL)) {
        b3Body_SetGravityScale(bodyId, 2);
        if (velocity.y > 0) b3Body_SetLinearVelocity(bodyId, b3Vec3(velocity.x, 0, velocity.z));
    }
    else {
        b3Body_SetGravityScale(bodyId, 1);
    }

    if (glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
    }

    b3Body_SetLinearVelocity(bodyId, b3Vec3(moveDirection.x * speed, b3Body_GetLinearVelocity(bodyId).y, moveDirection.z * speed));
    CapsuleBody3D::FixedUpdate(appState);
}

void Player3D::Event(AppState &appState, SDL_Event &event) {
    if (event.type == SDL_EVENT_MOUSE_MOTION && appState.isMouseRelative) {
        yaw->localTransform.rotate(glm::vec3(0, -event.motion.xrel * appState.sensitivity, 0));
        pitch->localTransform.rotate(glm::vec3(-event.motion.yrel * appState.sensitivity,0, 0));
    }

    if (event.button.button == SDL_BUTTON_RIGHT && appState.debug) {
        appState.isMouseRelative = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
        SDL_SetWindowRelativeMouseMode(appState.window, appState.isMouseRelative);
    }

    CapsuleBody3D::Event(appState, event);
}

bool Player3D::IsStandableSurface(const b3Vec3 normal) const
{
    const float maxSlopeCos = cosf( maxSlopeAngle * B3_PI / 180.0f );
    return b3Dot( normal, b3Vec3_axisY ) >= maxSlopeCos;
}

bool Player3D::IsGrounded(const AppState &appState) {
    const b3Pos pos = b3Body_GetPosition(bodyId);

    b3Pos from = pos;
    from.y += 0.015625;

    b3Pos to = pos;
    to.y -= 0.015625;

    const TraceResult tr = TraceCapsule(appState, from, to, radius, height);

    onGround = tr.hit && IsStandableSurface(tr.normal);
    return onGround;
}
