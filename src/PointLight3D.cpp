#include "PointLight3D.h"

#include "AppState.h"
#include "ImGuiWidgets.h"
#include "SDL3/SDL_log.h"
#include "thirdparty/imgui/imgui.h"

void PointLight3D::ImGuiDraw() {
    if (ImGui::CollapsingHeader("PointLight3D", ImGuiTreeNodeFlags_DefaultOpen)) {
        float _position[3] = { localTransform.position.x, localTransform.position.y, localTransform.position.z };
        ImGui::ColoredDragFloat3("##Position", _position, true);
        localTransform.position = glm::vec3(_position[0], _position[1], _position[2]);
        float col[3] = { color.x, color.y, color.z };
        ImGui::ColoredDragFloat3("RGB", col, false);
        color = glm::vec3(col[0], col[1], col[2]);
        ImGui::ColoredDragFloat("Intensity", &intensity, false);
    }
}

PointLight3D::PointLight3D(AppState* appState) {
    name = "PointLight3D";
    Register(appState);
}

void PointLight3D::Register(AppState* appState) {
    appState->pointLights.push_back(this);
    appState->CreatePointLightBuffer();
}

void PointLight3D::Unregister(AppState* appState) {
    if (const auto it = std::ranges::find(appState->pointLights, this); it != appState->pointLights.end()) {
        appState->pointLights.erase(it);
        appState->CreatePointLightBuffer();
    }
    else {
        SDL_Log("Attempted to unregister an unregistered PointLight3DGPU");
    }
}
