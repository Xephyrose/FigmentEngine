#include "Node3D.h"

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

Transform3D Node3D::GetGlobalTransform(double factor) const {
    if (parent == nullptr) {
        return localTransform;
    }

    const auto* par = dynamic_cast<Node3D*>(parent);
    if (par == nullptr) {
        return localTransform;
    }

    Transform3D parentWorld = par->GetGlobalTransformInterpolated(factor);

    // ----- 1. Build the quaternions directly from the Euler rotations -----
    //    This avoids any caching issues. If your 'rotation' is already
    //    in radians, replace glm::radians(rotation) with just rotation.
    glm::quat parentQuat = glm::quat(glm::radians(parentWorld.rotation));
    glm::quat localQuat  = glm::quat(glm::radians(localTransform.rotation));

    // ----- 2. Compose the transform correctly -----
    Transform3D world;

    // Position: scale the child's local offset by the parent's scale,
    //           then rotate that offset by the parent's orientation,
    //           then add to the parent's world position.
    // This makes the child orbit when the parent rotates.
    world.position = parentWorld.position
                   + parentQuat * (parentWorld.scale * localTransform.position);

    // Rotation: quaternion multiplication (order matters!).
    // Parent rotation is applied first, then the child's local rotation.
    world.quaternion = parentQuat * localQuat;

    // Scale: multiply component‑wise. Works correctly unless you have
    // deep non‑uniform scales combined with rotations (rare).
    world.scale = parentWorld.scale * localTransform.scale;

    // Keep the Euler angles up to date (optional, but convenient)
    world.rotation = glm::degrees(glm::eulerAngles(world.quaternion));

    return world;
}

Transform3D Node3D::GetGlobalTransformInterpolated(const double factor) const {
    return GetGlobalTransform(factor); // default Node3D is not interpolated, inherited structs may be though
}