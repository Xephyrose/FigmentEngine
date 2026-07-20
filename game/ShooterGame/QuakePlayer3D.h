#ifndef FIGMENTENGINE_QUAKEPLAYER3D_H
#define FIGMENTENGINE_QUAKEPLAYER3D_H
#include "Player3D.h"
#include "src/AppState.h"

struct QuakePlayer3D : Player3D {
    using Player3D::Player3D;
    void FixedUpdate(AppState& appState) override;
    void SV_AirMove(glm::vec3 &velocity, float delta) const;
    void SV_UserFriction(glm::vec3 &velocity, float delta) const;
    void SV_Accelerate(glm::vec3 &velocity, glm::vec3 wishDir, float wishSpeed, float delta) const;
    void SV_AirAccelerate(glm::vec3 &velocity, glm::vec3 wishDir, float wishSpeed, float delta) const;
    const float QUAKE_TO_GODOT_SCALE = 52.49f;
    const float sv_friction = 6.0f;                             // Quake: 6.0
    const float sv_stopspeed = 100.0f / QUAKE_TO_GODOT_SCALE;   // Quake: 100.0
    const float sv_maxspeed = 320.0f / QUAKE_TO_GODOT_SCALE;    // Quake: 320
    const float sv_accelerate = 10.0f;                          // Quake: 10.0
    const float cl_movespeed = 200.0f / QUAKE_TO_GODOT_SCALE;   // Quake: 200
    const float jump_impulse = sqrtf(2 * -39.2f * 0.5);
    glm::vec2 input_wishdir = glm::vec2(0);
    bool landing = false;
    bool landed = false;
    bool wish_jump = false;
    bool autobhop = false;
};


#endif //FIGMENTENGINE_QUAKEPLAYER3D_H
