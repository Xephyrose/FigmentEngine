#ifndef FIGMENTENGINE_GAME_H
#define FIGMENTENGINE_GAME_H
#include "src/Node.h"

struct Game : Node {
    Game();
    virtual void Init();
    void ImGuiDraw() override;
    void Update() override;
    void Draw(SDL_GPUCommandBuffer *commandBuffer) override;
    void DrawShadow(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass) override;
    void Input() override;
    void Event(SDL_Event &event) override;
};


#endif //FIGMENTENGINE_GAME_H
