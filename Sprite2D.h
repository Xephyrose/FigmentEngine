#ifndef FIGMENTENGINE_SPRITE2D_H
#define FIGMENTENGINE_SPRITE2D_H
#include "Node2D.h"

struct Sprite2D : Node2D {
    Sprite2D();
    std::string sprite;
    void Draw(AppState &appState, SDL_GPUCommandBuffer *commandBuffer) override;
};

#endif //FIGMENTENGINE_SPRITE2D_H
