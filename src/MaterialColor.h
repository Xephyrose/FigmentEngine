#ifndef FIGMENTENGINE_MATERIALCOLOR_H
#define FIGMENTENGINE_MATERIALCOLOR_H

#include "Material.h"

struct MaterialColor : Material {
    MaterialColor(AppState* appState, const std::string &name, const std::string &pipeline);
    glm::vec4 color = glm::vec4(1.0f);

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) override;

    void setColor(glm::vec4 color);
};

#endif //FIGMENTENGINE_MATERIALCOLOR_H
