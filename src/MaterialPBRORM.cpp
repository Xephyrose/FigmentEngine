#include "MaterialPBRORM.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"
#include "ImGuiWidgets.h"
#include "thirdparty/imgui/imgui_stdlib.h"

glm::vec3 SRGBToLinear(glm::vec3 c)
{
    const glm::vec3 low = c / 12.92f;
    const glm::vec3 high = glm::pow((c + 0.055f) / 1.055f, glm::vec3(2.4f));

    return glm::mix(low, high, glm::step(glm::vec3(0.04045f), c));
}

void MaterialPBRORM::ImGuiDraw() {
    ImGui::InputText("Texture", &texture);
    float col[4] = { color.x, color.y, color.z, color.w };
    ImGui::ColoredDragFloat4RGBA("RGBA", col);
    color = glm::vec4(col[0], col[1], col[2], col[3]);

    ImGui::InputText("ORM Map", &textureORM);
    ImGui::InputText("Normal Map", &textureNormalMap);
    ImGui::DragFloat("Metallic", &colorMetallic, 0.1f);
    ImGui::DragFloat("Roughness", &colorRoughness, 0.1f);
    ImGui::DragFloat("Ambient Occlusion", &colorAO, 0.1f);

    static const char* sampler_items[] = { "anisotropic_repeat", "linear_repeat", "linear_clamp", "nearest_repeat", "nearest_clamp" };
    static int sampler_selected_idx = 0;
    ImGui::Combo("Sampler", &sampler_selected_idx, sampler_items, IM_ARRAYSIZE(sampler_items));
    sampler = sampler_items[sampler_selected_idx];
}

void MaterialPBRORM::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, glm::mat4 model) {
    if (!appState->current_camera_3d) return;
    BindVertexUniformDataMMNL(*appState, commandBuffer, model);

    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    SDL_GPUTexture* getAlbedo = appState->GetTexture(texture);
    SDL_GPUTexture* getORM = appState->GetTexture(textureORM);
    SDL_GPUTexture* getNormal = appState->GetTexture(textureNormalMap);
    SDL_GPUSampler* getSamplerAlbedo = appState->GetSampler(sampler);
    SDL_GPUSampler* getSamplerORM = appState->GetSampler(sampler);
    SDL_GPUSampler* getSamplerNormalMap = appState->GetSampler(sampler);

    const SDL_GPUTextureSamplerBinding bindings[] = {
        {getAlbedo, getSamplerAlbedo},      // t0
        {getORM, getSamplerORM},           // t1
        {getNormal, getSamplerNormalMap}, // t2
        {appState->shadowMap, appState->GetSampler("shadow_sampler")} // t3
    };
    SDL_BindGPUFragmentSamplers(appState->renderPass, 0, bindings, std::size(bindings));

    struct PushData {
        glm::vec4 viewPos;
        glm::vec4 colorAlbedo;
        glm::uvec4 texturesUsed; // albedo, orm, normal
        glm::vec4 colorORM;
        glm::vec4 lightNums; // num_point_lights, num_dir_lights, num_spot_lights
    };
    PushData push{};
    push.viewPos = glm::vec4(appState->current_camera_3d->GetGlobalTransformInterpolated(appState->fixedTimeStepAccumulator / appState->fixedTimeStep).position, 0);
    push.colorAlbedo = glm::vec4(SRGBToLinear(color), color.w);
    push.texturesUsed.x = (texture == "none" ? 0 : 1);
    push.texturesUsed.y = (textureORM == "none" ? 0 : 1);
    push.texturesUsed.z = (textureNormalMap == "none" ? 0 : 1);
    push.colorORM.x = colorAO;
    push.colorORM.y = colorRoughness;
    push.colorORM.z = colorMetallic;
    push.lightNums.x = appState->pointLights.size();
    push.lightNums.y = appState->directionalLights.size();
    push.lightNums.z = appState->spotLights.size();

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &push, sizeof(PushData));

    SDL_BindGPUFragmentStorageBuffers(
        appState->renderPass,
        0,
        &appState->pointLightBuffer,
        1
    );

    SDL_BindGPUFragmentStorageBuffers(
        appState->renderPass,
        1,
        &appState->directionalLightBuffer,
        1
        );

    SDL_BindGPUFragmentStorageBuffers(
        appState->renderPass,
        2,
        &appState->spotLightBuffer,
        1
    );
}

void MaterialPBRORM::setTextureORM(AppState *appState, const std::string &texture) {
    textureORM = texture;
    if (textureORM != "none") {
        appState->LoadTexture(textureORM);
    }
}

void MaterialPBRORM::setTextureNormalMap(AppState *appState, const std::string &texture) {
    textureNormalMap = texture;
    if (textureNormalMap != "none") {
        appState->LoadTexture(textureNormalMap);
    }
}
