#pragma once

#include "../ECS/Component.h"
#include "../Math/Matrix4.h"
#include "../Math/Transform.h"

namespace GameEngine {
    class CameraComponent : public Component<CameraComponent> {
    public:
        CameraComponent() = default;
        CameraComponent(float fov, float nearPlane = 0.1f, float farPlane = 1000.0f)
            : fieldOfView(fov), nearPlane(nearPlane), farPlane(farPlane) {}
        
        float fieldOfView = 45.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        
        Matrix4 GetProjectionMatrix(float aspectRatio) const {
            return Matrix4::Perspective(fieldOfView * 3.14159f / 180.0f, aspectRatio, nearPlane, farPlane);
        }
        
        Matrix4 GetViewMatrix(const Transform& transform) const {
            Vector3 position = transform.GetPosition();
            Vector3 forward = transform.GetForward();
            Vector3 up = transform.GetUp();
            
            return Matrix4::LookAt(position, position + forward, up);
        }
    };
}
