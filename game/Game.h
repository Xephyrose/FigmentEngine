#ifndef FIGMENTENGINE_GAME_H
#define FIGMENTENGINE_GAME_H
#include "src/Node.h"

struct Game : Node {
    Game();
    void Init(AppState& appState);
    void ImGuiDraw() override;
    void Update(AppState& appState) override;
    void Draw(AppState &appState, SDL_GPUCommandBuffer *commandBuffer) override;
    void DrawShadow(AppState& appState, SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass) override;
    void Input(AppState& appState) override;
    void Event(AppState& appState, SDL_Event &event) override;
};


#endif //FIGMENTENGINE_GAME_H
