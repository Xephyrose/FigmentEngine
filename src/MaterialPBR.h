#ifndef FIGMENTENGINE_MATERIALPBR_H
#define FIGMENTENGINE_MATERIALPBR_H

#include "MaterialUnlitTextured.h"

struct MaterialPBR : MaterialUnlitTextured {
    using MaterialUnlitTextured::MaterialUnlitTextured;
    float colorMetallic = 0.5f;
    float colorRoughness = 0.5f;
    float colorAO = 1.0f;

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) override;

    void setColorMetallic(float color);
    void setColorRoughness(float color);
    void setColorAO(float color);
};

#endif //FIGMENTENGINE_MATERIALPBR_H
