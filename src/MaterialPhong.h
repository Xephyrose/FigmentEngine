#ifndef FIGMENTENGINE_MATERIALLIT_H
#define FIGMENTENGINE_MATERIALLIT_H

#include "Material.h"

struct MaterialPhong : Material {
    MaterialPhong(AppState* appState, const std::string &name, const std::string &pipeline);
    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) override;
};

#endif //FIGMENTENGINE_MATERIALLIT_H
