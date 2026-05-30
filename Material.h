#ifndef FIGMENTENGINE_MATERIAL_H
#define FIGMENTENGINE_MATERIAL_H
#include <string>

#include "AppState.h"

struct Material {
    std::pmr::string name;
    std::string pipeline;

    virtual ~Material() = default;
    virtual void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer) const = 0;
};

#endif //FIGMENTENGINE_MATERIAL_H