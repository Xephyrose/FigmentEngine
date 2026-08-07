#ifndef FIGMENTENGINE_MATERIAL_H
#define FIGMENTENGINE_MATERIAL_H
#include <string>

#include "AppState.h"

struct Material : Resource {
    std::string name;
    std::string pipeline;

    void ImGuiDraw() override;
    virtual void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) = 0;
    static void BindVertexUniformDataMMNL(const AppState &appState, SDL_GPUCommandBuffer *commandBuffer, const glm::mat4 &model);
};

#endif //FIGMENTENGINE_MATERIAL_H
