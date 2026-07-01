#include "Transform2D.h"

#include "ImGuiWidgets.h"
#include "SDL3/SDL_log.h"

void Transform2D::ImGuiDraw() {
    const char *xy[2] = {"X", "Y"};

    float _position[2] = { position.x, position.y };
    ImGui::ColoredDragFloat("##Position", _position, xy);
    position = glm::vec2(_position[0], _position[1]);

    float _scale[2] = { scale.x, scale.y };
    ImGui::ColoredDragFloat("##Scale", _scale, xy);
    scale = glm::vec2(_scale[0], _scale[1]);
}

void Transform2D::rotate(const float& eulerDegrees) {
    rotation += eulerDegrees;
}

glm::mat4 Transform2D::getMatrix() const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(scale, 1.0f));
    return model;
}

void Transform2D::move(const glm::vec2 amt) {
    position += amt;
}

void Transform2D::logTransform() const {
    SDL_Log("Transform2D: x: %f, y: %f, deg: %f", position.x, position.y, rotation);
}
