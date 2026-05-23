#ifndef FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H
#define FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>

#include "Material.h"

struct MaterialUnlitTextured : public Material {
    MaterialUnlitTextured(const std::string &name, const std::string &pipeline, const std::string& sampler, const std::string &textureAlbedo);
    std::string textureAlbedo;

    void Bind(AppState* appState) const override;
    void BindTextures(AppState* appState) const override;
};

#endif //FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H
