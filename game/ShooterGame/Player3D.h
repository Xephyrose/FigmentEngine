#ifndef FIGMENTENGINE_PLAYER3D_H
#define FIGMENTENGINE_PLAYER3D_H
#include "src/Camera3D.h"
#include "src/CapsuleBody3D.h"

struct Player3D : CapsuleBody3D {
    Player3D(AppState &appState, float pos_x, float pos_y, float pos_z, float radius, float height);
    void FixedUpdate(AppState& appState) override;
    void Event(AppState& appState, SDL_Event &event) override;
    [[nodiscard]] bool IsGrounded(const AppState &appState) const;
    [[nodiscard]] bool IsStandableSurface(b3Vec3 normal) const;
    const float height;
    const float radius;
    const float speed = 10.0f;
    const float maxSlopeAngle = 45;
    Camera3D* cam;
    Node3D* yaw;
    Node3D* pitch;
};


#endif //FIGMENTENGINE_PLAYER3D_H
