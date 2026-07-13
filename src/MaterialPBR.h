#ifndef FIGMENTENGINE_MATERIALPBR_H
#define FIGMENTENGINE_MATERIALPBR_H

#include "MaterialUnlitTextured.h"

struct MaterialPBR : MaterialUnlitTextured {
    using MaterialUnlitTextured::MaterialUnlitTextured;
    std::string textureMetallic = "none";
    float colorMetallic = 0.5f;
    std::string textureRoughness = "none";
    float colorRoughness = 0.5f;
    std::string textureAO = "none";
    float colorAO = 0.0f;

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) override;

    void setColorMetallic(float color);
    void setTextureMetallic(AppState* appState, const std::string &texture);
    void setColorRoughness(float color);
    void setTextureRoughness(AppState* appState, const std::string &texture);
    void setColorAO(float color);
    void setTextureAO(AppState* appState, const std::string &texture);
};

#endif //FIGMENTENGINE_MATERIALPBR_H
