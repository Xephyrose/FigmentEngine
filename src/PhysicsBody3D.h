#ifndef FIGMENTENGINE_PHYSICSBODY3D_H
#define FIGMENTENGINE_PHYSICSBODY3D_H
#include "Node3D.h"
#include "box3d/box3d.h"

struct PhysicsBody3D : Node3D {
    PhysicsBody3D(AppState &appState, b3BodyType bodyType, float pos_x, float pos_y, float pos_z);

    b3BodyId bodyId{};

    [[nodiscard]] Transform3D GetGlobalTransform() const override;
    void Update(AppState& appState) override;
};

#endif //FIGMENTENGINE_PHYSICSBODY3D_H
