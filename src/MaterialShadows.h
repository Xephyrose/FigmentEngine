#ifndef FIGMENTENGINE_MATERIALSHADOWS_H
#define FIGMENTENGINE_MATERIALSHADOWS_H
#include "Material.h"


struct MaterialShadows : Material {
    MaterialShadows(AppState* appState, const std::string &name, const std::string &pipeline);

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) override;
};


#endif //FIGMENTENGINE_MATERIALSHADOWS_H
