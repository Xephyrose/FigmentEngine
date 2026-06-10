#ifndef FIGMENTENGINE_POINTLIGHT3D_H
#define FIGMENTENGINE_POINTLIGHT3D_H
#include "Node3D.h"
#include "PointLight3DGPU.h"

struct PointLight3D : public Node3D {
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;

    explicit PointLight3D(AppState &appState);
    void Register(AppState &appState);
    void Unregister(AppState &appState);
};

#endif //FIGMENTENGINE_POINTLIGHT3D_H
