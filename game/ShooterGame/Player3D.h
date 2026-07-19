#ifndef FIGMENTENGINE_PLAYER3D_H
#define FIGMENTENGINE_PLAYER3D_H
#include "src/CapsuleBody3D.h"

struct Player3D : CapsuleBody3D {
    Player3D(AppState &appState, float pos_x, float pos_y, float pos_z, float radius, float height);
    void FixedUpdate(AppState& appState) override;
    const float speed = 1.0f;
};


#endif //FIGMENTENGINE_PLAYER3D_H
