#pragma once

#include "../ECS/Component.h"
#include "../Math/Transform.h"

namespace GameEngine {
    class TransformComponent : public Component<TransformComponent> {
    public:
        TransformComponent() = default;
        TransformComponent(const Vector3& position, const Quaternion& rotation = Quaternion::Identity(), const Vector3& scale = Vector3::One)
            : transform(position, rotation, scale) {}
        
        Transform transform;
    };
}
