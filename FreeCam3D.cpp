#include "FreeCam3D.h"

#include <SDL3/SDL_scancode.h>

#include "AppState.h"
#include "Input.h"

void FreeCam3D::Input(const AppState& appState) {
    const glm::vec3 forward = localTransform.getForward();
    const glm::vec3 right = localTransform.getRight();

    auto moveDirection = glm::vec3(0.0f);
    if (Input::IsPressed(SDL_SCANCODE_W)) moveDirection += forward;
    if (Input::IsPressed(SDL_SCANCODE_S)) moveDirection -= forward;
    if (Input::IsPressed(SDL_SCANCODE_D)) moveDirection += right;
    if (Input::IsPressed(SDL_SCANCODE_A)) moveDirection -= right;
    if (Input::IsPressed(SDL_SCANCODE_SPACE)) moveDirection += UP;
    if (Input::IsPressed(SDL_SCANCODE_LCTRL)) moveDirection -= UP;

    if (glm::length(moveDirection) > 0.0f) {
        moveDirection = glm::normalize(moveDirection);
    }

    localTransform.move(moveDirection * 0.025f * static_cast<float>(appState.delta));
}