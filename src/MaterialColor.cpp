#include "MaterialColor.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"
#include "ImGuiWidgets.h"

MaterialColor::MaterialColor(AppState *appState, const std::string &name, const std::string &pipeline) {
    this->name = name;
    this->pipeline = pipeline;
    appState->materials.insert_or_assign(name, this);
}

void MaterialColor::ImGuiDraw() {
    float col[4] = { color.x, color.y, color.z, color.w };
    ImGui::ColoredDragFloat4RGBA("RGBA", col);
    color = glm::vec4(col[0], col[1], col[2], col[3]);
}

void MaterialColor::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, glm::mat4 model) {
    if (!appState->current_camera_3d) return;
    BindVertexUniformDataMMNL(*appState, commandBuffer, model);

    SDL_GPUGraphicsPipeline* gotPipeline = appState->GetPipeline(pipeline);
    if (!gotPipeline) {
        SDL_Log("ERROR: Pipeline '%s' not found!", pipeline.c_str());
        return;
    }

    SDL_BindGPUGraphicsPipeline(appState->renderPass, gotPipeline);

    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &color, sizeof(glm::vec4));
}

void MaterialColor::setColor(glm::vec4 col) {
    color = col;
}
