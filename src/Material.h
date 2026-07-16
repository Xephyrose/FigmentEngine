#ifndef FIGMENTENGINE_MATERIAL_H
#define FIGMENTENGINE_MATERIAL_H
#include <string>

#include "AppState.h"

struct Material {
    std::string name;
    std::string pipeline;

    virtual ~Material() = default;
    virtual void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) = 0;

    static void BindVertexUniformDataMMNL(const AppState *appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model);
};

#endif //FIGMENTENGINE_MATERIAL_H
