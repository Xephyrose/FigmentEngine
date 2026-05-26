#ifndef FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H
#define FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H

#include "Material.h"

struct MaterialUnlitTextured : public Material {
    MaterialUnlitTextured(AppState* appState, const std::string &name, const std::string &pipeline, const std::string& sampler, const std::string &textureAlbedo);
    std::string textureAlbedo;
    glm::vec4 colorAlbedo = glm::vec4(0.5f);

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer) const override;
};

#endif //FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H
