#pragma once

#include "../ECS/Component.h"
#include "../Math/Matrix4.h"
#include "../Math/Transform.h"
#include "../Logging/Logger.h"
#include <algorithm>

namespace GameEngine {
    class CameraComponent : public Component<CameraComponent> {
    public:
        CameraComponent() = default;
        CameraComponent(float fov, float nearPlane = 0.1f, float farPlane = 1000.0f)
            : fieldOfView(fov), nearPlane(nearPlane), farPlane(farPlane) {}
        
        float fieldOfView = 45.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        
        void SetFOV(float fov) {
            fieldOfView = std::clamp(fov, 10.0f, 170.0f);
        }
        
        float GetFOV() const { 
            return fieldOfView; 
        }
        
        void AdjustFOV(float delta) {
            SetFOV(fieldOfView + delta);
        }
        
        Matrix4 GetProjectionMatrix(float aspectRatio) const {
            float fovRadians = fieldOfView * 3.14159f / 180.0f;
            Logger::Debug("Camera: Creating projection matrix - FOV=" + std::to_string(fieldOfView) + "°, aspect=" + std::to_string(aspectRatio) + ", near=" + std::to_string(nearPlane) + ", far=" + std::to_string(farPlane));
            return Matrix4::Perspective(fovRadians, aspectRatio, nearPlane, farPlane);
        }
        
        Matrix4 GetViewMatrix(const Transform& transform) const {
            Vector3 position = transform.GetPosition();
            
            // Look directly at the center of the cube grid (cubes are positioned around origin at y=0)
            Vector3 target = Vector3(0.0f, 0.0f, 0.0f);  // Center of cube grid
            Vector3 up = Vector3::Up;  // World up vector
            
            Logger::Debug("Camera position: (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + ")");
            Logger::Debug("Camera target: (" + std::to_string(target.x) + ", " + std::to_string(target.y) + ", " + std::to_string(target.z) + ")");
            Logger::Debug("Camera up: (" + std::to_string(up.x) + ", " + std::to_string(up.y) + ", " + std::to_string(up.z) + ")");
            
            // Calculate direction vector for verification
            Vector3 direction = (target - position).Normalized();
            Logger::Debug("Camera direction: (" + std::to_string(direction.x) + ", " + std::to_string(direction.y) + ", " + std::to_string(direction.z) + ")");
            
            return Matrix4::LookAt(position, target, up);
        }
    };
}
