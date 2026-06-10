#ifndef FIGMENTENGINE_POINTLIGHT3D_H
#define FIGMENTENGINE_POINTLIGHT3D_H
#include "Node3D.h"

struct PointLight3D : Node3D {
    void Draw(AppState &appState, SDL_GPUCommandBuffer *commandBuffer) override;
    void Update(AppState &appState) override;
};

#endif //FIGMENTENGINE_POINTLIGHT3D_H
