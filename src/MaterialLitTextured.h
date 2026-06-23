#ifndef FIGMENTENGINE_MATERIALLITTEXTURED_H
#define FIGMENTENGINE_MATERIALLITTEXTURED_H

#include "MaterialUnlitTextured.h"

struct MaterialLitTextured : MaterialUnlitTextured {
    using MaterialUnlitTextured::MaterialUnlitTextured;
    float shininess = 64;
    std::string textureAmbient = "none";
    glm::vec3 colorAmbient = glm::vec3(0);
    std::string textureSpecular = "none";
    glm::vec3 colorSpecular = glm::vec3(1.0f);

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer) override;
};


#endif //FIGMENTENGINE_MATERIALLITTEXTURED_H
