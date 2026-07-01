#ifndef FIGMENTENGINE_DIRECTIONALLIGHT3D_H
#define FIGMENTENGINE_DIRECTIONALLIGHT3D_H
#include "Node3D.h"

struct DirectionalLight3D : Node3D {
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;

    void ImGuiDraw() override;
    explicit DirectionalLight3D(AppState* appState);
    void Register(AppState* appState);
    void Unregister(AppState* appState);
};


#endif //FIGMENTENGINE_DIRECTIONALLIGHT3D_H
