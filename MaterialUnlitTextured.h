#ifndef FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H
#define FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H

#include "Material.h"

struct MaterialUnlitTextured : public Material {
    MaterialUnlitTextured(AppState* appState, const std::string &name, const std::string &pipeline);
    std::string textureAlbedo;
    std::string samplerAlbedo;
    glm::vec4 colorAlbedo = glm::vec4(1.0f);

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer) const override;

    void setColorAlbedo(glm::vec4 color);
    void setTextureAlbedo(AppState* appState, const std::string &texture);
    void setSamplerAlbedo(AppState* appState, const std::string &sampler);
};

#endif //FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H
