#ifndef FIGMENTENGINE_MATERIALPBRORM_H
#define FIGMENTENGINE_MATERIALPBRORM_H
#include "MaterialPBR.h"

struct MaterialPBRORM : MaterialPBR{
    using MaterialPBR::MaterialPBR;
    std::string textureORM = "none";

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) override;

    void setTextureORM(AppState* appState, const std::string &texture);
};

#endif //FIGMENTENGINE_MATERIALPBRORM_H
