#ifndef FIGMENTENGINE_MATERIALPHONGTEXTUREDHEIGHT_H
#define FIGMENTENGINE_MATERIALPHONGTEXTUREDHEIGHT_H

#include "Material.h"

struct BlinnPhongMaterialLayer {
    glm::vec3 colorAlbedo = glm::vec3(1.0f);
    glm::vec3 colorAmbient = glm::vec3(0.25f);
    glm::vec3 colorSpecular = glm::vec3(1.0f);
    float shininess = 64;
};

struct MaterialPhongTexturedHeight : Material {
    MaterialPhongTexturedHeight(AppState* appState, const std::string &name, const std::string &pipeline, const std::vector<BlinnPhongMaterialLayer> &initial_layers);
    std::vector<BlinnPhongMaterialLayer> layers;
    // these must be textures with layers, as you can't create a Texture2DArray from regular GPUTextures
    std::string textureAlbedo = "none";
    std::string textureAmbient = "none";
    std::string textureSpecular = "none";
    std::string textureNormalMap = "none";
    std::string sampler = "anisotropic_repeat";

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) override;

    void setSampler(AppState* appState, const std::string &new_sampler);
};

#endif //FIGMENTENGINE_MATERIALPHONGTEXTUREDHEIGHT_H
