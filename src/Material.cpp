#include "Material.h"

#include "Camera3D.h"
#include "thirdparty/imgui/imgui.h"
#include "../thirdparty/imgui/imgui_stdlib.h"

void Material::ImGuiDraw() {
    if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputText("Pipeline", &pipeline);
    }
}

void Material::BindVertexUniformDataMMNL(const AppState &appState, SDL_GPUCommandBuffer *commandBuffer, const glm::mat4 &model) {
    const glm::mat4 view = appState.current_camera_3d->GetViewMatrixInterpolated(appState.fixedTimeStepAccumulator / appState.fixedTimeStep);
    const glm::mat4 proj = appState.current_camera_3d->GetProjectionMatrix(appState.currentAspectRatio);
    const glm::mat4 mvp = proj * view * model;
    const glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));

    struct TransformData {
        glm::mat4 mvp;
        glm::mat4 model;
        glm::mat4 normalMatrix;
        glm::mat4 lightVP;
    };

    TransformData data{};
    data.mvp = mvp;
    data.model = model;
    data.normalMatrix = normalMatrix;
    data.lightVP = appState.GetOffsetLightViewProjection();

    SDL_PushGPUVertexUniformData(commandBuffer, 0, &data, sizeof(data));
}
