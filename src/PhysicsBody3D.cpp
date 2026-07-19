#include "PhysicsBody3D.h"

#include "AppState.h"

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
    bodyDef.position = (b3Vec3){pos_x / 50.0f, pos_y / 50.0f, pos_z / 50.0f};
    bodyId = b3CreateBody(appState.worldId3, &bodyDef);
}

Transform3D PhysicsBody3D::GetGlobalTransform() const {
    return localTransform;
}

void PhysicsBody3D::Update(AppState &appState) {
    localTransform.quaternion = ToGLM(b3Body_GetRotation(bodyId));
    localTransform.position.x = b3Body_GetPosition(bodyId).x * 50;
    localTransform.position.y = b3Body_GetPosition(bodyId).y * 50;
    localTransform.position.z = b3Body_GetPosition(bodyId).z * 50;
    // localTransform.logTransform();
    Node3D::Update(appState);
}
