#ifndef FIGMENTENGINE_CAMERA2D_H
#define FIGMENTENGINE_CAMERA2D_H
#include "Node2D.h"

struct Camera2D : Node2D {
    Camera2D();
    void ImGuiDraw() override;
    float zoom = 1.0f;

    [[nodiscard]] glm::mat4 GetViewMatrix() const;
    [[nodiscard]] glm::mat4 GetProjectionMatrix(float screenWidth, float screenHeight) const;
};

#endif //FIGMENTENGINE_CAMERA2D_H
