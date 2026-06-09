#ifndef FIGMENTENGINE_SPRITE2D_H
#define FIGMENTENGINE_SPRITE2D_H
#include "Node2D.h"

struct Sprite2D : Node2D {
    Sprite2D();
    void ImGuiDraw() override;
    std::string sprite;
    glm::vec2 size;
    void Draw(AppState &appState, SDL_GPUCommandBuffer *commandBuffer) override;
};

#endif //FIGMENTENGINE_SPRITE2D_H
