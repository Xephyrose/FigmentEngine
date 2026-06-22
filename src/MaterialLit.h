#ifndef FIGMENTENGINE_MATERIALLIT_H
#define FIGMENTENGINE_MATERIALLIT_H

#include "Material.h"

struct MaterialLit : Material {
    MaterialLit(AppState* appState, const std::string &name, const std::string &pipeline);
    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer) override;
};

#endif //FIGMENTENGINE_MATERIALLIT_H
