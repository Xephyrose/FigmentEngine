#include "FreeCam.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_scancode.h>

void FreeCam::Input(const AppState& appState, const bool* isKeyDown) {
    // Get the camera's forward and right vectors from its rotation
    glm::vec3 forward = appState.current_camera->localTransform.getForward();
    glm::vec3 right = appState.current_camera->localTransform.getRight();
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); // Or get from camera if you have roll

    // Build movement vector from input
    auto moveDirection = glm::vec3(0.0f);
    if (isKeyDown[SDL_SCANCODE_W]) moveDirection += forward;
    if (isKeyDown[SDL_SCANCODE_S]) moveDirection -= forward;
    if (isKeyDown[SDL_SCANCODE_D]) moveDirection += right;
    if (isKeyDown[SDL_SCANCODE_A]) moveDirection -= right;
    if (isKeyDown[SDL_SCANCODE_SPACE]) moveDirection += up;
    if (isKeyDown[SDL_SCANCODE_LCTRL]) moveDirection -= up;

    // Normalize to prevent faster diagonal movement
    if (glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
    }

    // Move the camera
    appState.current_camera->localTransform.move(moveDirection * 0.1f);
}