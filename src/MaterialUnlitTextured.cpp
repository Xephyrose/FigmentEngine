#include "MaterialUnlitTextured.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"
#include "ImGuiWidgets.h"
#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/imgui_stdlib.h"

MaterialUnlitTextured::MaterialUnlitTextured(const std::string &name, const std::string &pipeline) {
    this->name = name;
    this->pipeline = pipeline;
    AppState::Get().materials.insert_or_assign(name, this);
}

void MaterialUnlitTextured::ImGuiDraw() {
    ImGui::InputText("Texture", &texture); // TODO: because we don't call setTextureAlbedo, this may not be safe if called on a bad texture

    static const char* sampler_items[] = { "anisotropic_repeat", "linear_repeat", "linear_clamp", "nearest_repeat", "nearest_clamp" };
    static int sampler_selected_idx = 0;
    ImGui::Combo("Sampler", &sampler_selected_idx, sampler_items, IM_ARRAYSIZE(sampler_items));
    sampler = sampler_items[sampler_selected_idx];

    float col[4] = { color.x, color.y, color.z, color.w };
    ImGui::ColoredDragFloat4RGBA("RGBA", col);
    color = glm::vec4(col[0], col[1], col[2], col[3]);
}

void MaterialUnlitTextured::Bind(AppState *appState, SDL_GPUCommandBuffer* commandBuffer, const glm::mat4 model) {
    if (!appState->current_camera_3d) return;
    BindVertexUniformDataMMNL(*appState, commandBuffer, model);

    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    SDL_GPUTexture* getAlbedo = appState->GetTexture(texture);
    // SDL_GPUTexture* getAlbedo = appState->shadowMap;
    SDL_GPUSampler* getSampler = appState->GetSampler(sampler);

    const SDL_GPUTextureSamplerBinding binding = {getAlbedo, getSampler};
    SDL_BindGPUFragmentSamplers(appState->renderPass, 0, &binding, 1);

    struct PushData {
        glm::vec4 colorAlbedo;
        bool      useTexture;
    };
    PushData push{};
    push.colorAlbedo = color;
    push.useTexture = (texture == "none" ? 0 : 1);

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &push, sizeof(PushData));
}

void MaterialUnlitTextured::setColorAlbedo(const glm::vec4 new_color) {
    color = new_color;
}

void MaterialUnlitTextured::setTextureAlbedo(const std::string &new_texture) {
    texture = new_texture;
    if (texture != "none") {
        AppState::Get().LoadTexture(texture);
    }
}

void MaterialUnlitTextured::setSampler(const std::string &new_sampler) {
    sampler = new_sampler;
}
