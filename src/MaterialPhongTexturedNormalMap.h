#ifndef FIGMENTENGINE_MATERIALPHONGTEXTUREDNORMALMAP_H
#define FIGMENTENGINE_MATERIALPHONGTEXTUREDNORMALMAP_H
#include "MaterialPhongTextured.h"

struct MaterialPhongTexturedNormalMap : MaterialPhongTextured {
    using MaterialPhongTextured::MaterialPhongTextured;
    std::string textureNormalMap = "none";

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) override;

    void setTextureNormalMap(AppState* appState, const std::string &texture);
};


#endif //FIGMENTENGINE_MATERIALPHONGTEXTUREDNORMALMAP_H
