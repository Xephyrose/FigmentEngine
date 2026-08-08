#ifndef FIGMENTENGINE_MATERIALHEIGHTMAP_H
#define FIGMENTENGINE_MATERIALHEIGHTMAP_H

#include "MaterialUnlitTextured.h"

struct MaterialHeightMap : MaterialUnlitTextured {
    using MaterialUnlitTextured::MaterialUnlitTextured;

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) override;
};


#endif //FIGMENTENGINE_MATERIALHEIGHTMAP_H
