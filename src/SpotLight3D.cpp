#include "SpotLight3D.h"

#include "AppState.h"
#include "ImGuiWidgets.h"
#include "SDL3/SDL_log.h"
#include "thirdparty/imgui/imgui.h"

void SpotLight3D::ImGuiDraw() {
    if (ImGui::CollapsingHeader("SpotLight3D", ImGuiTreeNodeFlags_DefaultOpen)) {
        float _position[3] = { localTransform.position.x, localTransform.position.y, localTransform.position.z };
        ImGui::ColoredDragFloat3XYZ("##Position", _position);
        localTransform.position = glm::vec3(_position[0], _position[1], _position[2]);
        float _rotation[3] = { localTransform.rotation.x, localTransform.rotation.y, localTransform.rotation.z };
        ImGui::ColoredDragFloat3XYZ("##Rotation", _rotation);
        localTransform.rotation = glm::vec3(_rotation[0], _rotation[1], _rotation[2]);
        float col[3] = { color.x, color.y, color.z };
        ImGui::ColoredDragFloat3RGB("RGB", col);
        color = glm::vec3(col[0], col[1], col[2]);
        // ImGui::Text("Brightness");
        // ImGui::SameLine();
        // ImGui::ColoredDragFloat("Brightness", &brightness, nullptr);
        // ImGui::Text("Constant");
        // ImGui::SameLine();
        // ImGui::ColoredDragFloat("Constant", &constant, false);
        // ImGui::Text("Linear");
        // ImGui::SameLine();
        // ImGui::ColoredDragFloat("Linear", &linear, false);
        // ImGui::Text("Quadratic");
        // ImGui::SameLine();
        // ImGui::ColoredDragFloat("Quadratic", &quadratic, false);
        // ImGui::Text("Inner Falloff");
        // ImGui::SameLine();
        // ImGui::ColoredDragFloat("Inner Falloff", &cutoff, false);
        // ImGui::Text("Outer Falloff");
        // ImGui::SameLine();
        // ImGui::ColoredDragFloat("Outer Falloff", &outerCutoff, false);
    }
}

SpotLight3D::SpotLight3D(AppState* appState) {
    name = "SpotLight3D";
    Register(appState);
}

void SpotLight3D::Register(AppState* appState) {
    appState->spotLights.push_back(this);
    appState->CreateSpotLightBuffer();
}

void SpotLight3D::Unregister(AppState* appState) {
    if (const auto it = std::ranges::find(appState->spotLights, this); it != appState->spotLights.end()) {
        appState->spotLights.erase(it);
        appState->CreateSpotLightBuffer();
    }
    else {
        SDL_Log("Attempted to unregister an unregistered SpotLight3DGPU");
    }
}
