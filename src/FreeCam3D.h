#ifndef FIGMENTENGINE_FREECAM_H
#define FIGMENTENGINE_FREECAM_H
#include "Camera3D.h"

struct FreeCam3D : Camera3D {
    FreeCam3D();
    void ImGuiDraw() override;
    void Update(AppState& appState) override;
    void Input(AppState& appState) override;
    void Event(AppState& appState, SDL_Event &event) override;
    float speed = 0.025f;
    glm::vec3 moveDirection = glm::vec3(0.0f);
};

#endif //FIGMENTENGINE_FREECAM_H
