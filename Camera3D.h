#ifndef FIGMENTENGINE_CAMERA3D_H
#define FIGMENTENGINE_CAMERA3D_H
#include "Node3D.h"

class Camera3D : public Node3D {
public:
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    [[nodiscard]] glm::mat4 GetViewMatrix() const;
    [[nodiscard]] glm::mat4 GetProjectionMatrix(float aspectRatio) const;
};


#endif //FIGMENTENGINE_CAMERA3D_H
