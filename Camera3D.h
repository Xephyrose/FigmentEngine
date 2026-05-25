#ifndef FIGMENTENGINE_CAMERA3D_H
#define FIGMENTENGINE_CAMERA3D_H
#include "Node3D.h"

struct Camera3D : public Node3D {
public:
    explicit Camera3D(const AppState& appState);
    float fov = 90.0f;
    float nearPlane = 0.1f;
    float farPlane = 500.0f;

    [[nodiscard]] glm::mat4 GetViewMatrix() const;
    [[nodiscard]] glm::mat4 GetProjectionMatrix(float aspectRatio) const;
};


#endif //FIGMENTENGINE_CAMERA3D_H
