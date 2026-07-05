#ifndef FIGMENTENGINE_MESHINSTANCE3D_H
#define FIGMENTENGINE_MESHINSTANCE3D_H
#include "Node3D.h"

struct MeshInstance3D : Node3D {
    MeshInstance3D();
    std::string mesh;
    void Draw(AppState &appState, SDL_GPUCommandBuffer *commandBuffer) override;
    void DrawShadow(AppState &appState, SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass* renderPass) override;
};

#endif //FIGMENTENGINE_MESHINSTANCE3D_H
