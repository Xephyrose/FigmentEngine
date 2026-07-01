#ifndef FIGMENTENGINE_POINTLIGHT3D_H
#define FIGMENTENGINE_POINTLIGHT3D_H
#include "Node3D.h"

struct PointLight3D : Node3D {
    glm::vec3 color = glm::vec3(1.0f);
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    void ImGuiDraw() override;
    explicit PointLight3D(AppState* appState);
    void Register(AppState* appState);
    void Unregister(AppState* appState);
};

#endif //FIGMENTENGINE_POINTLIGHT3D_H
