#ifndef FIGMENTENGINE_DYNAMICBODY2D_H
#define FIGMENTENGINE_DYNAMICBODY2D_H
#include "Node2D.h"
#include "box2d/box2d.h"

struct PhysicsBody2D : Node2D {
    PhysicsBody2D(b2BodyType bodyType, float size_x, float size_y, float pos_x, float pos_y);

    b2BodyId bodyId{};

    [[nodiscard]] Transform2D GetGlobalTransform() const override;
    void Update() override;
};

#endif //FIGMENTENGINE_DYNAMICBODY2D_H
