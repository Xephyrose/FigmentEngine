#include "DirectionalLight3D.h"

#include "AppState.h"
#include "ImGuiWidgets.h"
#include "SDL3/SDL_log.h"
#include "thirdparty/imgui/imgui.h"

void DirectionalLight3D::ImGuiDraw() {
    if (ImGui::CollapsingHeader("DirectionalLight3D", ImGuiTreeNodeFlags_DefaultOpen)) {
        float _rotation[3] = { localTransform.rotation.x, localTransform.rotation.y, localTransform.rotation.z };
        ImGui::ColoredDragFloat3("##Position", _rotation, true);
        localTransform.rotation = glm::vec3(_rotation[0], _rotation[1], _rotation[2]);
        float col[3] = { color.x, color.y, color.z };
        ImGui::ColoredDragFloat3("RGB", col, false);
        color = glm::vec3(col[0], col[1], col[2]);
        ImGui::ColoredDragFloat("Intensity", &intensity, false);
    }
}

DirectionalLight3D::DirectionalLight3D(AppState* appState) {
    name = "DirectionalLight3D";
    Register(appState);
}

void DirectionalLight3D::Register(AppState* appState) {
    appState->directionalLights.push_back(this);
}

void DirectionalLight3D::Unregister(AppState* appState) {
    if (const auto it = std::ranges::find(appState->directionalLights, this); it != appState->directionalLights.end()) {
        appState->directionalLights.erase(it);
    }
    else {
        SDL_Log("Attempted to unregister an unregistered DirectionalLight3DGPU");
    }
}
