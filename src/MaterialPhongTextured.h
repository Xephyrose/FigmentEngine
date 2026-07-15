#ifndef FIGMENTENGINE_MATERIALLITTEXTURED_H
#define FIGMENTENGINE_MATERIALLITTEXTURED_H

#include "MaterialUnlitTextured.h"

struct MaterialPhongTextured : MaterialUnlitTextured {
    using MaterialUnlitTextured::MaterialUnlitTextured;
    float shininess = 64;
    std::string textureAmbient = "none";
    glm::vec3 colorAmbient = glm::vec3(0.25f);
    std::string textureSpecular = "none";
    glm::vec3 colorSpecular = glm::vec3(1.0f);
    std::string textureNormalMap = "none";

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) override;

    void setColorAmbient(glm::vec4 color);
    void setTextureAmbient(AppState* appState, const std::string &texture);
    void setColorSpecular(glm::vec4 color);
    void setTextureSpecular(AppState* appState, const std::string &texture);

    virtual void setTextureNormalMap(AppState* appState, const std::string &texture);
};


#endif //FIGMENTENGINE_MATERIALLITTEXTURED_H
