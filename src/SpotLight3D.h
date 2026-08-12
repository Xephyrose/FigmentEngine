#ifndef FIGMENTENGINE_SPOTLIGHT3D_H
#define FIGMENTENGINE_SPOTLIGHT3D_H
#include "Node3D.h"

struct SpotLight3D : Node3D {
    glm::vec3 color = glm::vec3(1.0f);
    float brightness = 1.0f;
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    float cutoff = 0.0f;
    float outerCutoff = 15.0f;

    void ImGuiDraw() override;
    explicit SpotLight3D();
    void Register();
    void Unregister();
};


#endif //FIGMENTENGINE_SPOTLIGHT3D_H
