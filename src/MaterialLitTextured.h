#ifndef FIGMENTENGINE_MATERIALLITTEXTURED_H
#define FIGMENTENGINE_MATERIALLITTEXTURED_H

#include "MaterialUnlitTextured.h"

struct MaterialLitTextured : MaterialUnlitTextured {
    using MaterialUnlitTextured::MaterialUnlitTextured;
    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer) override;
};


#endif //FIGMENTENGINE_MATERIALLITTEXTURED_H
