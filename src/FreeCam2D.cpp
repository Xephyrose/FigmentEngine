#include "FreeCam2D.h"
#include "AppState.h"
#include "../thirdparty/imgui/imgui.h"

FreeCam2D::FreeCam2D() {
    name = "FreeCam2D";
}

void FreeCam2D::ImGuiDraw() {
    Camera2D::ImGuiDraw();
}

void FreeCam2D::Event(SDL_Event &event) {
    AppState* appState = &AppState::Get();
    if (event.type == SDL_EVENT_MOUSE_MOTION && appState->isMouseRelative) {
        localTransform.move(glm::vec2(-event.motion.xrel, -event.motion.yrel));
    }

    if (event.button.button == SDL_BUTTON_RIGHT && appState->debug) {
        appState->isMouseRelative = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
        SDL_SetWindowRelativeMouseMode(appState->window, appState->isMouseRelative);
    }

    Camera2D::Event(event);
}
