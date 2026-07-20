#ifndef FIGMENTENGINE_GLMHELPER_H
#define FIGMENTENGINE_GLMHELPER_H
#include "box3d/box3d.h"
#include "box3d/id.h"
#include "thirdparty/glm/vec3.hpp"

inline glm::vec3 GetLinearVelocity(const b3BodyId bodyId) {
    const auto [x, y, z] = b3Body_GetLinearVelocity(bodyId);
    return glm::vec3{x, y, z};
}

inline void SetLinearVelocity(const b3BodyId bodyId, const glm::vec3 velocity) {
    const auto linearVelocity = b3Vec3{velocity.x, velocity.y, velocity.z};
    b3Body_SetLinearVelocity(bodyId, linearVelocity);
}

#endif //FIGMENTENGINE_GLMHELPER_H
