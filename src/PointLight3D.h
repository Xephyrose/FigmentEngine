#ifndef FIGMENTENGINE_POINTLIGHT3D_H
#define FIGMENTENGINE_POINTLIGHT3D_H
#include "Node3D.h"

struct PointLight3D : Node3D {
    glm::vec3 color = glm::vec3(1.0f);
    float constant;
    float linear;
    float quadratic;

    void ImGuiDraw() override;
    explicit PointLight3D(AppState* appState);
    void Register(AppState* appState);
    void Unregister(AppState* appState);
};

#endif //FIGMENTENGINE_POINTLIGHT3D_H
