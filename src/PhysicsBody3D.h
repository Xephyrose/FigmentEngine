#ifndef FIGMENTENGINE_PHYSICSBODY3D_H
#define FIGMENTENGINE_PHYSICSBODY3D_H
#include "Node3D.h"
#include "box3d/box3d.h"

struct TraceResult
{
    b3Pos endPosition;
    b3Vec3 normal;
    b3Pos hitPoint;
    float fraction;
    bool hit;
    bool startedSolid;
};

struct ClosestShapeCastContext
{
    b3ShapeId ignoreShapes[16];
    int ignoreCount;
    float closestFraction;
    b3Vec3 closestNormal;
    b3Pos closestPoint;
    b3ShapeId closestShape;
    bool hit;
    bool startedSolid;
};

struct PhysicsBody3D : Node3D {
    PhysicsBody3D(AppState &appState, b3BodyType bodyType, float pos_x, float pos_y, float pos_z);

    static float ClosestShapeCastCallback(b3ShapeId _shapeId, b3Pos point, b3Vec3 normal, float fraction, uint64_t userMaterialId, int triangleIndex, int childIndex, void* context);
    [[nodiscard]] TraceResult TraceCapsule(const AppState &appState, b3Pos from, b3Pos to, float radius, float height) const;

    b3BodyId bodyId{};
    b3ShapeId shapeId{};
    Transform3D last_tick_transform;

    void FixedUpdate(AppState& appState) override;
    [[nodiscard]] Transform3D GetGlobalTransform() const override;
protected:
    [[nodiscard]] Transform3D GetGlobalTransformInterpolatedREAL(double factor) const override;
};

#endif //FIGMENTENGINE_PHYSICSBODY3D_H
