#ifndef FIGMENTENGINE_MATERIAL2D_H
#define FIGMENTENGINE_MATERIAL2D_H
#include "MaterialUnlitTextured.h"

struct Material2D : MaterialUnlitTextured {
    using MaterialUnlitTextured::MaterialUnlitTextured;
    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) override;
};


#endif //FIGMENTENGINE_MATERIAL2D_H
