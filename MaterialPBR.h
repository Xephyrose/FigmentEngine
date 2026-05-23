#ifndef FIGMENTENGINE_MATERIALPBR_H
#define FIGMENTENGINE_MATERIALPBR_H

#include <array>
#include <SDL3/SDL_gpu.h>

#include "Material.h"

struct MaterialPBR : public Material {
    static constexpr int TEXTURE_COUNT = 4;
    enum TextureSlot { ALBEDO = 0, NORMAL = 1, ROUGHNESS = 2, METALLIC = 3 };

    std::array<SDL_GPUTexture*, TEXTURE_COUNT> textures = {nullptr};
    SDL_GPUSampler* sampler = nullptr;

    void Bind(AppState* appState) const override {
        SDL_BindGPUGraphicsPipeline(appState->renderPass, appState->GetPipeline(pipeline));
        BindTextures(appState);
    }

    void BindTextures(AppState* appState) const override {
        std::array<SDL_GPUTextureSamplerBinding, TEXTURE_COUNT> bindings;
        for (int i = 0; i < TEXTURE_COUNT; ++i) {
            bindings[i] = {textures[i] ? textures[i] : nullptr, sampler};
        }
        SDL_BindGPUFragmentSamplers(appState->renderPass, 0, bindings.data(), TEXTURE_COUNT);
    }
};

#endif //FIGMENTENGINE_MATERIALPBR_H
