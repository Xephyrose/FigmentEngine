#ifndef FIGMENTENGINE_TRANSFORM_H
#define FIGMENTENGINE_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform3D {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 rotation{0.0f};
    glm::quat quaternion{1.0f, 0.0f, 0.0f, 0.0f};

    bool rotationDirty{true};
    bool usingQuaternion{false};

    void updateQuaternion();
    void updateEuler();
    void setRotation(const glm::vec3& eulerDegrees);
    void setQuaternion(const glm::quat& q);
    const glm::quat& getQuaternion();
    const glm::vec3& getEuler();
    void rotate(const glm::vec3& eulerDegrees);

    void rotateX(float degrees);
    void rotateY(float degrees);
    void rotateZ(float degrees);

    void rotateObjectLocal(const glm::vec3& eulerDegrees);
    [[nodiscard]] glm::mat4 getMatrix() const;
    [[nodiscard]] glm::vec3 getForward() const;
    [[nodiscard]] glm::vec3 getRight() const;
    [[nodiscard]] glm::vec3 getUp() const;
    void lookAt(const glm::vec3& target);
};

inline constexpr glm::vec3 ZERO = { 0, 0, 0 };
inline constexpr glm::vec3 LEFT = { -1, 0, 0 };
inline constexpr glm::vec3 RIGHT = { 1, 0, 0 };
inline constexpr glm::vec3 UP = { 0, 1, 0 };
inline constexpr glm::vec3 DOWN = { 0, -1, 0 };
inline constexpr glm::vec3 FORWARD = { 0, 0, -1 };
inline constexpr glm::vec3 BACK = { 0, 0, 1 };

#endif //FIGMENTENGINE_TRANSFORM_H
