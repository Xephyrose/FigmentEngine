#include "Node3D.h"

#include "SDL3/SDL_log.h"
#include "thirdparty/imgui/imgui.h"

Node3D::Node3D() {
    name = "Node3D";
}

void Node3D::ImGuiDraw() {
    Node::ImGuiDraw();
    if (ImGui::CollapsingHeader("Node3D", ImGuiTreeNodeFlags_DefaultOpen)) {
        localTransform.ImGuiDraw();
    }
}

Transform3D Node3D::GetGlobalTransformInterpolated(const double factor) const {
    return GetGlobalTransformInterpolatedREAL(factor);
}

Transform3D Node3D::GetGlobalTransform() const {
    // sonion I got NO IDEA how or why the actual fuck this shit works, but I fucked with it long enough.
    // good luck o7
    if (parent == nullptr) {
        return localTransform;
    }

    const auto* par = dynamic_cast<Node3D*>(parent);
    if (par == nullptr) {
        return localTransform;
    }

    Transform3D parentGlobalTransform = par->GetGlobalTransform();
    auto parentQuat = parentGlobalTransform.quaternion;
    auto localQuat  = localTransform.quaternion;

    Transform3D globalTransform;
    globalTransform.position = parentGlobalTransform.position + parentQuat * (parentGlobalTransform.scale * localTransform.position);
    globalTransform.quaternion = parentQuat * localQuat;
    globalTransform.scale = parentGlobalTransform.scale * localTransform.scale;
    globalTransform.rotation = glm::degrees(glm::eulerAngles(globalTransform.quaternion));
    return globalTransform;
}

Transform3D Node3D::GetGlobalTransformInterpolatedREAL(double factor) const {
    // falls back to regular transform, override this function in derived structs
    // default Node3D is not interpolated, inherited structs may be though
    if (parent == nullptr) {
        return localTransform;
    }

    const auto* par = dynamic_cast<Node3D*>(parent);
    if (par == nullptr) {
        return localTransform;
    }

    Transform3D parentGlobalTransform = par->GetGlobalTransformInterpolatedREAL(factor);
    auto parentQuat = parentGlobalTransform.quaternion;
    auto localQuat  = localTransform.quaternion;

    Transform3D globalTransform;
    globalTransform.position = parentGlobalTransform.position + parentQuat * (parentGlobalTransform.scale * localTransform.position);
    globalTransform.quaternion = parentQuat * localQuat;
    globalTransform.scale = parentGlobalTransform.scale * localTransform.scale;
    globalTransform.rotation = glm::degrees(glm::eulerAngles(globalTransform.quaternion));

    SDL_Log("current=%f name=%s",
            globalTransform.position.x,
            name.c_str());

    return globalTransform;
}
