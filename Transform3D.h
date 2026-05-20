#ifndef FIGMENTENGINE_TRANSFORM_H
#define FIGMENTENGINE_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform3D {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    glm::mat4 GetModelMatrix() const;
};

inline constexpr glm::vec3 ZERO = { 0, 0, 0 };
inline constexpr glm::vec3 LEFT = { -1, 0, 0 };
inline constexpr glm::vec3 RIGHT = { 1, 0, 0 };
inline constexpr glm::vec3 UP = { 0, 1, 0 };
inline constexpr glm::vec3 DOWN = { 0, -1, 0 };
inline constexpr glm::vec3 FORWARD = { 0, 0, -1 };
inline constexpr glm::vec3 BACK = { 0, 0, 1 };

#endif //FIGMENTENGINE_TRANSFORM_H
