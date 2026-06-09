#include "Transform3D.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "thirdparty/imgui/imgui.h"

void Transform3D::ImGuiDraw() {
    float _position[3] = { position.x, position.y, position.z };
    if (ImGui::InputFloat3("Position", _position)) {
        position = glm::vec3(_position[0], _position[1], _position[2]);
    }

    float _rotation[3] = { rotation.x, rotation.y, rotation.z };
    if (ImGui::InputFloat3("Rotation", _rotation)) {
        setRotation(glm::vec3(_rotation[0], _rotation[1], _rotation[2]));
    }

    float _scale[3] = { scale.x, scale.y, scale.z };
    if (ImGui::InputFloat3("Scale", _scale)) {
        scale = glm::vec3(_scale[0], _scale[1], _scale[2]);
    }
}

void Transform3D::updateQuaternion() {
    quaternion = glm::quat(glm::radians(rotation));
    rotationDirty = false;
    usingQuaternion = false;
}

void Transform3D::updateEuler() {
    rotation = glm::degrees(glm::eulerAngles(quaternion));
    rotationDirty = false;
    usingQuaternion = true;
}

void Transform3D::setRotation(const glm::vec3& eulerDegrees) {
    rotation = eulerDegrees;
    updateQuaternion();
}

void Transform3D::setQuaternion(const glm::quat& q) {
    quaternion = glm::normalize(q);
    rotationDirty = true;
    usingQuaternion = true;
}

const glm::quat& Transform3D::getQuaternion() {
    if (rotationDirty && !usingQuaternion) updateQuaternion();
    else if (rotationDirty) updateEuler();
    return quaternion;
}

const glm::vec3& Transform3D::getEuler() {
    if (rotationDirty && usingQuaternion) updateEuler();
    else if (rotationDirty) updateQuaternion();
    return rotation;
}

void Transform3D::rotate(const glm::vec3& eulerDegrees) {
    if (rotationDirty) updateEuler();
    rotation += eulerDegrees;
    updateQuaternion();
}

void Transform3D::rotateX(float degrees) { rotate(glm::vec3(degrees, 0, 0)); }
void Transform3D::rotateY(float degrees) { rotate(glm::vec3(0, degrees, 0)); }
void Transform3D::rotateZ(float degrees) { rotate(glm::vec3(0, 0, degrees)); }

void Transform3D::rotateObjectLocal(const glm::vec3& eulerDegrees) {
    if (rotationDirty) updateQuaternion();
    glm::quat rotX = glm::angleAxis(glm::radians(eulerDegrees.x), glm::vec3(1, 0, 0));
    glm::quat rotY = glm::angleAxis(glm::radians(eulerDegrees.y), glm::vec3(0, 1, 0));
    glm::quat rotZ = glm::angleAxis(glm::radians(eulerDegrees.z), glm::vec3(0, 0, 1));

    quaternion = glm::normalize(quaternion * rotX * rotY * rotZ);
    rotationDirty = true;
    usingQuaternion = true;
}

glm::mat4 Transform3D::getMatrix() const {
    glm::mat4 mat(1.0f);
    mat = glm::translate(mat, position);

    if (!rotationDirty || usingQuaternion) {
        mat = mat * glm::mat4_cast(quaternion);
    } else {
        mat = mat * glm::mat4_cast(glm::quat(glm::radians(rotation)));
    }

    mat = glm::scale(mat, scale);
    return mat;
}

glm::vec3 Transform3D::getForward() const {
    const glm::quat& q = (!rotationDirty || usingQuaternion) ?
        quaternion : glm::quat(glm::radians(rotation));
    return q * glm::vec3(0, 0, -1);
}

glm::vec3 Transform3D::getRight() const {
    const glm::quat& q = (!rotationDirty || usingQuaternion) ?
        quaternion : glm::quat(glm::radians(rotation));
    return q * glm::vec3(1, 0, 0);
}

glm::vec3 Transform3D::getUp() const {
    const glm::quat& q = (!rotationDirty || usingQuaternion) ?
        quaternion : glm::quat(glm::radians(rotation));
    return q * glm::vec3(0, 1, 0);
}

void Transform3D::lookAt(const glm::vec3& target) {
    const glm::mat4 lookAtMat = glm::lookAt(position, target, UP);
    const glm::mat3 rotMat(lookAtMat);
    quaternion = glm::normalize(glm::quat_cast(glm::transpose(rotMat)));
    rotationDirty = true;
    usingQuaternion = true;
}

void Transform3D::move(glm::vec3 amt) {
    position += amt;
}