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
            
            // Safety check: ensure forward vector is not zero
            if (forward.LengthSquared() < 0.0001f) {
                forward = Vector3::Forward; // Default forward direction
            }
            
            // Safety check: ensure up vector is not zero
            if (up.LengthSquared() < 0.0001f) {
                up = Vector3::Up; // Default up direction
            }
            
            Vector3 target = position + forward;
            
            // Safety check: ensure camera is not looking at itself
            if ((target - position).LengthSquared() < 0.0001f) {
                target = position + Vector3::Forward;
            }
            
            return Matrix4::LookAt(position, target, up);
        }
    };
}
