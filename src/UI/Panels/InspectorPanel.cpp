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

void InspectorPanel::Update(World* world, float deltaTime) {
    if (!m_visible || !world) return;
    
    ImGui::Begin("Inspector", &m_visible);
    
    if (m_selectedEntity.IsValid() && world->IsEntityValid(m_selectedEntity)) {
        ImGui::Text("Entity %u", m_selectedEntity.GetID());
        ImGui::Separator();
        
        DrawTransformComponent(world, m_selectedEntity);
        DrawCameraComponent(world, m_selectedEntity);
        DrawMovementComponent(world, m_selectedEntity);
        
        ImGui::Separator();
        
        if (ImGui::Button("Add Camera Component")) {
            if (!world->HasComponent<CameraComponent>(m_selectedEntity)) {
                world->AddComponent<CameraComponent>(m_selectedEntity, 45.0f);
                Logger::Info("Added CameraComponent to entity " + std::to_string(m_selectedEntity.GetID()));
            }
        }
        
        if (ImGui::Button("Add Movement Component")) {
            if (!world->HasComponent<MovementComponent>(m_selectedEntity)) {
                world->AddComponent<MovementComponent>(m_selectedEntity, 5.0f);
                Logger::Info("Added MovementComponent to entity " + std::to_string(m_selectedEntity.GetID()));
            }
        }
    } else {
        ImGui::Text("No entity selected");
    }
    
    ImGui::End();
}

void InspectorPanel::DrawTransformComponent(World* world, Entity entity) {
    auto* transform = world->GetComponent<TransformComponent>(entity);
    if (!transform) return;
    
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        Vector3 position = transform->transform.GetPosition();
        Vector3 rotation = transform->transform.GetRotation().ToEulerAngles();
        Vector3 scale = transform->transform.GetScale();
        
        rotation = rotation * (180.0f / 3.14159f);
        
        if (ImGui::DragFloat3("Position", &position.x, 0.1f)) {
            transform->transform.SetPosition(position);
        }
        
        if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f)) {
            Vector3 rotationRad = rotation * (3.14159f / 180.0f);
            Quaternion quat = Quaternion::FromEulerAngles(rotationRad.x, rotationRad.y, rotationRad.z);
            transform->transform.SetRotation(quat);
        }
        
        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f)) {
            transform->transform.SetScale(scale);
        }
    }
}

void InspectorPanel::DrawCameraComponent(World* world, Entity entity) {
    auto* camera = world->GetComponent<CameraComponent>(entity);
    if (!camera) return;
    
    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::DragFloat("Field of View", &camera->fieldOfView, 1.0f, 1.0f, 179.0f);
        ImGui::DragFloat("Near Plane", &camera->nearPlane, 0.01f, 0.01f, camera->farPlane - 0.01f);
        ImGui::DragFloat("Far Plane", &camera->farPlane, 1.0f, camera->nearPlane + 0.01f, 10000.0f);
        
        if (ImGui::Button("Remove Camera Component")) {
            world->RemoveComponent<CameraComponent>(entity);
            Logger::Info("Removed CameraComponent from entity " + std::to_string(entity.GetID()));
        }
    }
}

void InspectorPanel::DrawMovementComponent(World* world, Entity entity) {
    auto* movement = world->GetComponent<MovementComponent>(entity);
    if (!movement) return;
    
    if (ImGui::CollapsingHeader("Movement")) {
        ImGui::DragFloat("Movement Speed", &movement->movementSpeed, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Mouse Sensitivity", &movement->mouseSensitivity, 0.1f, 0.1f, 10.0f);
        
        ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", movement->velocity.x, movement->velocity.y, movement->velocity.z);
        ImGui::Text("Pitch: %.2f, Yaw: %.2f", movement->pitch * 180.0f / 3.14159f, movement->yaw * 180.0f / 3.14159f);
        
        if (ImGui::Button("Remove Movement Component")) {
            world->RemoveComponent<MovementComponent>(entity);
            Logger::Info("Removed MovementComponent from entity " + std::to_string(entity.GetID()));
        }
    }
}

}
