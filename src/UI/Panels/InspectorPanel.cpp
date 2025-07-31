#include "InspectorPanel.h"
#include "../../Core/ECS/World.h"
#include "../../Core/Components/TransformComponent.h"
#include "../../Core/Components/CameraComponent.h"
#include "../../Core/Components/MovementComponent.h"
#include "../../Core/Logging/Logger.h"
#include <imgui.h>

namespace GameEngine {

InspectorPanel::InspectorPanel() = default;

InspectorPanel::~InspectorPanel() = default;

void InspectorPanel::Update(World* world, float /*deltaTime*/) {
    if (!m_visible || !world) return;
    
    if (m_selectedEntity.IsValid() && world->IsEntityValid(m_selectedEntity)) {
        Logger::Debug("Inspector showing entity ID: " + std::to_string(m_selectedEntity.GetID()) + " (simplified mode)");
        
        DrawTransformComponent(world, m_selectedEntity);
        DrawCameraComponent(world, m_selectedEntity);
        DrawMovementComponent(world, m_selectedEntity);
    } else {
        Logger::Debug("Inspector: No entity selected (simplified mode)");
    }
}

void InspectorPanel::DrawTransformComponent(World* world, Entity entity) {
    auto* transform = world->GetComponent<TransformComponent>(entity);
    if (!transform) return;
    
    Vector3 position = transform->transform.GetPosition();
    Vector3 rotation = transform->transform.GetRotation().ToEulerAngles();
    Vector3 scale = transform->transform.GetScale();
    
    (void)rotation;
    (void)scale;
    
    Logger::Debug("Transform Component - Pos: (" + std::to_string(position.x) + ", " + 
                  std::to_string(position.y) + ", " + std::to_string(position.z) + ") (simplified mode)");
}

void InspectorPanel::DrawCameraComponent(World* world, Entity entity) {
    auto* camera = world->GetComponent<CameraComponent>(entity);
    if (!camera) return;
    
    Logger::Debug("Camera Component - FOV: " + std::to_string(camera->fieldOfView) + 
                  ", Near: " + std::to_string(camera->nearPlane) + 
                  ", Far: " + std::to_string(camera->farPlane) + " (simplified mode)");
}

void InspectorPanel::DrawMovementComponent(World* world, Entity entity) {
    auto* movement = world->GetComponent<MovementComponent>(entity);
    if (!movement) return;
    
    Logger::Debug("Movement Component - Speed: " + std::to_string(movement->movementSpeed) + 
                  ", Sensitivity: " + std::to_string(movement->mouseSensitivity) + " (simplified mode)");
}

}
