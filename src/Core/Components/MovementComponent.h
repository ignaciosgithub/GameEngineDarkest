#pragma once

#include "../ECS/Component.h"
#include "../Math/Vector3.h"

namespace GameEngine {
    class MovementComponent : public Component<MovementComponent> {
    public:
        MovementComponent() = default;
        MovementComponent(float speed, float sensitivity = 2.0f)
            : movementSpeed(speed), mouseSensitivity(sensitivity) {}
        
        float movementSpeed = 5.0f;
        float mouseSensitivity = 0.5f;
        
        Vector3 velocity = Vector3::Zero;
        
        // Mouse look state
        float pitch = 0.0f;
        float yaw = 0.0f;
        bool firstMouse = true;
        Vector3 lastMousePos = Vector3::Zero;
    };
}
