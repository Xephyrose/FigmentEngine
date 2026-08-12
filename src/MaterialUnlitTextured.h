#ifndef FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H
#define FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H

#include "Material.h"

struct MaterialUnlitTextured : Material {
    MaterialUnlitTextured(const std::string &name, const std::string &pipeline);
    void ImGuiDraw() override;
    std::string texture = "none";
    std::string sampler = "anisotropic_repeat";
    glm::vec4 color = glm::vec4(1.0f);

    void Bind(AppState* appState, SDL_GPUCommandBuffer* commandBuffer, glm::mat4 model) override;

    void setColorAlbedo(glm::vec4 color);
    void setTextureAlbedo(const std::string &texture);
    void setSampler(const std::string &new_sampler);
};

#endif //FIGMENTENGINE_MATERIAL_UNLIT_TEXTURED_H
