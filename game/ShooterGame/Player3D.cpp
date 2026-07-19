#include "Player3D.h"

#include "../../src/Input.h"
#include "SDL3/SDL_log.h"
#include "src/AppState.h"
#include "src/Camera3D.h"

#include "src/MeshInstance3D.h"

Player3D::Player3D(AppState &appState, const float pos_x, const float pos_y, const float pos_z, float radius, float height) : CapsuleBody3D(appState, b3_dynamicBody, pos_x, pos_y, pos_z, radius, height) {
    name = "Player3D";
    b3MotionLocks locks = {};
    locks.angularX = true;
    locks.angularY = true;
    locks.angularZ = true;
    b3Body_SetMotionLocks(bodyId, locks);

    yaw = new Node3D();
    yaw->name = "Yaw";
    yaw->localTransform.position = glm::vec3(0.0f, 2.0f, 0.0f);
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
    meshInstance2->localTransform.rotation = glm::vec3(0.0f, 180, 0.0f);
    cam->addChild(std::unique_ptr<Node>(meshInstance2));
}

void Player3D::FixedUpdate(AppState &appState) {
    // Keep in mind this is the velocity at the START of the method
    auto [velX, velY, velZ] = b3Body_GetLinearVelocity(bodyId);
    if (Input::IsJustPressed(SDL_SCANCODE_SPACE)) {
        // b3Body_SetLinearVelocity(bodyId, b3Vec3(velX, 12, velZ));
        b3Body_SetLinearVelocity(bodyId, b3Vec3(velX, 0, velZ));
        b3Body_ApplyLinearImpulseToCenter(bodyId, b3Vec3(0, 1100, 0), true);
    }

    const glm::vec3 forward = yaw->localTransform.getForward();
    const glm::vec3 right = yaw->localTransform.getRight();

    moveDirection = glm::vec3(0.0f);
    if (Input::IsPressed(SDL_SCANCODE_W)) moveDirection += forward;
    if (Input::IsPressed(SDL_SCANCODE_S)) moveDirection -= forward;
    if (Input::IsPressed(SDL_SCANCODE_D)) moveDirection += right;
    if (Input::IsPressed(SDL_SCANCODE_A)) moveDirection -= right;
    if (Input::IsPressed(SDL_SCANCODE_SPACE)) moveDirection += UP;
    if (Input::IsPressed(SDL_SCANCODE_LCTRL)) moveDirection -= UP;


    if (Input::IsPressed(SDL_SCANCODE_LCTRL)) {
        b3Body_SetGravityScale(bodyId, 2);
        if (velY > 0) b3Body_SetLinearVelocity(bodyId, b3Vec3(velX, 0, velZ));
    }
    else {
        b3Body_SetGravityScale(bodyId, 1);
    }

    if (glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
    }

    b3Body_SetLinearVelocity(bodyId, b3Vec3(moveDirection.x * speed, b3Body_GetLinearVelocity(bodyId).y, moveDirection.z * speed));
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
