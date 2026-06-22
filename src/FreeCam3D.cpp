#include "FreeCam3D.h"

#include <SDL3/SDL_scancode.h>

#include "AppState.h"
#include "../thirdparty/imgui/imgui.h"
#include "Input.h"

FreeCam3D::FreeCam3D() {
    name = "FreeCam3D";
}

void FreeCam3D::ImGuiDraw() {
    Camera3D::ImGuiDraw();
    ImGui::Text("Node");
    ImGui::InputFloat("Speed", &speed);
}

void FreeCam3D::Input(AppState& appState) {
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

    localTransform.move(moveDirection * speed * static_cast<float>(appState.delta));
    Camera3D::Input(appState);
}

void FreeCam3D::Event(AppState &appState, SDL_Event &event) {
    if (event.type == SDL_EVENT_MOUSE_MOTION && appState.isMouseRelative) {
        localTransform.rotate(glm::vec3(-event.motion.yrel * appState.sensitivity, -event.motion.xrel * appState.sensitivity, 0));
    }

    if (event.button.button == SDL_BUTTON_RIGHT && appState.debug) {
        appState.isMouseRelative = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
        SDL_SetWindowRelativeMouseMode(appState.window, appState.isMouseRelative);
    }

    Camera3D::Event(appState, event);
}
