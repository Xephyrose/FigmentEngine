#ifndef FIGMENTENGINE_FREECAM_H
#define FIGMENTENGINE_FREECAM_H
#include "Camera3D.h"

class FreeCam : public Camera3D {
public:
    using Camera3D::Camera3D;
    void Input(const AppState& appState, const bool* isKeyDown) override;
};

#endif //FIGMENTENGINE_FREECAM_H
