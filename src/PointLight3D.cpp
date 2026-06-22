#include "PointLight3D.h"

#include "AppState.h"
#include "SDL3/SDL_log.h"
#include "thirdparty/imgui/imgui.h"

void PointLight3D::ImGuiDraw() {
    Node3D::ImGuiDraw();
    ImGui::Text("PointLight3D");
    float col[3] = { color.x, color.y, color.z };
    if (ImGui::DragFloat3("RGB", col)) {
        color = glm::vec3(col[0], col[1], col[2]);
    }
    ImGui::DragFloat("Intensity", &intensity);
}

PointLight3D::PointLight3D(AppState* appState) {
    name = "PointLight3D";
    Register(appState);
}

void PointLight3D::Register(AppState* appState) {
    appState->pointLights.push_back(this);
}

void PointLight3D::Unregister(AppState* appState) {
    if (const auto it = std::ranges::find(appState->pointLights, this); it != appState->pointLights.end()) {
        appState->pointLights.erase(it);
    }
    else {
        SDL_Log("Attempted to unregister an unregistered PointLight3DGPU");
    }
}
