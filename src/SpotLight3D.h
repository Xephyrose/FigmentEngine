#ifndef FIGMENTENGINE_SPOTLIGHT3D_H
#define FIGMENTENGINE_SPOTLIGHT3D_H
#include "Node3D.h"

struct SpotLight3D : Node3D {
    glm::vec3 color = glm::vec3(1.0f);
    float constant;
    float linear;
    float quadratic;
    float cutoff = 1.0f;
    float outerCutoff = 1.0f;

    void ImGuiDraw() override;
    explicit SpotLight3D(AppState* appState);
    void Register(AppState* appState);
    void Unregister(AppState* appState);
};


#endif //FIGMENTENGINE_SPOTLIGHT3D_H
