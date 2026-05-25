#ifndef FIGMENTENGINE_FREECAM_H
#define FIGMENTENGINE_FREECAM_H
#include "Camera3D.h"

struct FreeCam : public Camera3D {
    using Camera3D::Camera3D;
    void Input(const AppState& appState, const bool* isKeyDown) override;
};

#endif //FIGMENTENGINE_FREECAM_H
