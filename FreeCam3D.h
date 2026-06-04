#ifndef FIGMENTENGINE_FREECAM_H
#define FIGMENTENGINE_FREECAM_H
#include "Camera3D.h"

struct FreeCam3D : public Camera3D {
    FreeCam3D();
    void ImGuiDraw() override;
    void Input(AppState& appState) override;
    void Event(AppState& appState, SDL_Event &event) override;
    float speed = 0.025f;
};

#endif //FIGMENTENGINE_FREECAM_H
