#ifndef FIGMENTENGINE_MESHINSTANCE3D_H
#define FIGMENTENGINE_MESHINSTANCE3D_H
#include "Node3D.h"

struct MeshInstance3D : public Node3D {
    std::string mesh;
    void Draw(AppState &appState, SDL_GPUCommandBuffer *commandBuffer) override;
};

#endif //FIGMENTENGINE_MESHINSTANCE3D_H
