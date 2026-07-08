#ifndef FIGMENTENGINE_PLAYER2D_H
#define FIGMENTENGINE_PLAYER2D_H
#include "../../src/PhysicsBody2D.h"

struct Player2D : PhysicsBody2D {
    Player2D(AppState &appState, float size_x, float size_y, float pos_x, float pos_y);
    void FixedUpdate(AppState& appState) override;
    const float speed = 10.0f;
};

#endif //FIGMENTENGINE_PLAYER2D_H
