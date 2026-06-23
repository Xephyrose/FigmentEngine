#ifndef FIGMENTENGINE_TRANSFORM2D_H
#define FIGMENTENGINE_TRANSFORM2D_H

#include <thirdparty/glm/glm.hpp>
#include <thirdparty/glm/gtc/matrix_transform.hpp>
#include <thirdparty/glm/gtc/quaternion.hpp>

struct Transform2D {
    void ImGuiDraw();

    glm::vec2 position{0.0f, 0.0f};
    glm::vec2 scale{1.0f, 1.0f};
    float rotation{0.0f};

    void rotate(const float& eulerDegrees);
    [[nodiscard]] glm::mat4 getMatrix() const;

    void move(glm::vec2 amt);
    void logTransform() const;
};


#endif //FIGMENTENGINE_TRANSFORM2D_H
