#include "PhysicsBody3D.h"

#include "AppState.h"
#include "SDL3/SDL_log.h"

inline glm::quat ToGLM(const b3Quat& q)
{
    return {
        q.s,      // w
        q.v.x,    // x
        q.v.y,    // y
        q.v.z     // z
    };
}

inline b3Quat ToB3(const glm::quat& q)
{
    return b3Quat{
            { q.x, q.y, q.z }, q.w
        };
}

PhysicsBody3D::PhysicsBody3D(AppState &appState, b3BodyType bodyType, float pos_x, float pos_y, float pos_z) {
    name = "PhysicsBody3D";
    b3BodyDef bodyDef = b3DefaultBodyDef();
    bodyDef.type = bodyType;
    bodyDef.position = (b3Vec3){pos_x, pos_y, pos_z};
    bodyId = b3CreateBody(appState.worldId3, &bodyDef);
}

Transform3D PhysicsBody3D::GetGlobalTransform() const {
    return localTransform;
}

Transform3D PhysicsBody3D::GetGlobalTransformInterpolatedREAL(double factor) const {
    factor = std::clamp(factor, 0.0, 1.0);
    // interpolate between last_tick_transform and localTransform by a factor of factor
    Transform3D interpolated;

    interpolated.position = glm::mix(last_tick_transform.position, localTransform.position, static_cast<float>(factor));
    interpolated.quaternion = glm::slerp(last_tick_transform.quaternion, localTransform.quaternion, static_cast<float>(factor));
    interpolated.rotation = glm::mix(last_tick_transform.rotation, localTransform.rotation, static_cast<float>(factor));

    return interpolated;
}

void PhysicsBody3D::FixedUpdate(AppState &appState) {
    last_tick_transform = localTransform;
    Node3D::FixedUpdate(appState);
}

void PhysicsBody3D::PostPhysicsUpdate(AppState &appState) {
    localTransform.setQuaternion(ToGLM(b3Body_GetRotation(bodyId)));
    localTransform.setPosition(b3Body_GetPosition(bodyId));
    // localTransform.logTransform();
    Node3D::PostPhysicsUpdate(appState);
}

TraceResult PhysicsBody3D::TraceCapsule(const AppState &appState, const b3Pos from, const b3Pos to, const float radius, const float height) const {
    TraceResult result = {};
    result.endPosition = to;
    result.normal = b3Vec3_axisY;
    result.hitPoint = to;
    result.fraction = 1.0f;

    const b3Vec3 translation = to - from;
    if (b3Length(translation) < 1e-6f) return result;

    b3Vec3 points[2];

    points[0] = {0.0f, (height * 0.5f - radius), 0.0f};
    points[1] = {0.0f, -(height * 0.5f - radius), 0.0f};

    const b3ShapeProxy proxy{points,2, radius};

    ClosestShapeCastContext context = {};
    context.closestFraction = 1.0f;

    context.ignoreShapes[0] = shapeId;
    context.ignoreCount = 1;

    const b3QueryFilter filter = b3DefaultQueryFilter();
    b3World_CastShape( appState.worldId3, from, &proxy, translation, filter, ClosestShapeCastCallback, &context );

    result.startedSolid = context.startedSolid;
    if ( context.hit )
    {
        result.hit = true;
        result.fraction = context.closestFraction;
        result.normal = context.closestNormal;
        result.hitPoint = context.closestPoint;
        result.endPosition = from + context.closestFraction * translation;
    }

    return result;
}

float PhysicsBody3D::ClosestShapeCastCallback(const b3ShapeId _shapeId, const b3Pos point, const b3Vec3 normal, const float fraction, uint64_t userMaterialId, int triangleIndex, int childIndex, void* context)
{
    auto* ctx = static_cast<ClosestShapeCastContext*>( context );

    for ( int i = 0; i < ctx->ignoreCount; ++i )
    {
        if ( B3_ID_EQUALS( _shapeId, ctx->ignoreShapes[i] ) )
        {
            return -1.0f;
        }
    }

    if ( fraction <= 0.0f )
    {
        ctx->startedSolid = true;
        return -1.0f;
    }

    if ( fraction < ctx->closestFraction )
    {
        ctx->closestFraction = fraction;
        ctx->closestNormal = normal;
        ctx->closestPoint = point;
        ctx->closestShape = _shapeId;
        ctx->hit = true;
    }

    return ctx->closestFraction;
}
