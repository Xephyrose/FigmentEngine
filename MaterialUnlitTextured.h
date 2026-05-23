#ifndef FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H
#define FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>

#include "Material.h"

struct MaterialUnlitTextured : public Material {
    MaterialUnlitTextured(const std::string &name, const std::string &pipeline, const std::string &textureAlbedo);
    std::string textureAlbedo;

    void Bind(AppState* appState) const override {
        SDL_Log("test");
        SDL_BindGPUGraphicsPipeline(appState->renderPass, appState->GetPipeline(pipeline));
        SDL_Log("test 2");
        BindTextures(appState);
    }

    void BindTextures(AppState* appState) const override {
        SDL_GPUTexture* getAlbedo = appState->GetTexture(textureAlbedo);
        SDL_GPUSampler* getSampler = appState->GetSampler(textureAlbedo);


        if (getAlbedo && getSampler) {
            const SDL_GPUTextureSamplerBinding binding = {getAlbedo, getSampler};
            SDL_BindGPUFragmentSamplers(appState->renderPass, 0, &binding, 1);
        }
    }
};

#endif //FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H
