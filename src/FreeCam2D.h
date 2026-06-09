#ifndef FIGMENTENGINE_FREECAM2D_H
#define FIGMENTENGINE_FREECAM2D_H
#include "Camera2D.h"

struct FreeCam2D : public Camera2D {
    FreeCam2D();
    void ImGuiDraw() override;
    void Event(AppState& appState, SDL_Event &event) override;
};


#endif //FIGMENTENGINE_FREECAM2D_H
