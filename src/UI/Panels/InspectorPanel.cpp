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
    
    if (ImGui::Begin("Inspector", &m_visible)) {
        if (m_selectedEntity.IsValid() && world->IsEntityValid(m_selectedEntity)) {
            ImGui::Text("Entity ID: %d", m_selectedEntity.GetID());
            ImGui::Separator();
            
            DrawTransformComponent(world, m_selectedEntity);
            DrawCameraComponent(world, m_selectedEntity);
            DrawMovementComponent(world, m_selectedEntity);
        } else {
            ImGui::Text("No entity selected");
        }
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
        
        if (ImGui::DragFloat3("Position", &position.x, 0.1f)) {
            transform->transform.SetPosition(position);
        }
        if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f)) {
            transform->transform.SetRotation(Quaternion::FromEulerAngles(rotation.x, rotation.y, rotation.z));
        }
        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f)) {
            transform->transform.SetScale(scale);
        }
    }
}

void InspectorPanel::DrawCameraComponent(World* world, Entity entity) {
    auto* camera = world->GetComponent<CameraComponent>(entity);
    if (!camera) return;
    
    if (ImGui::CollapsingHeader("Camera")) {
        float fov = camera->fieldOfView;
        float nearPlane = camera->nearPlane;
        float farPlane = camera->farPlane;
        
        if (ImGui::SliderFloat("Field of View", &fov, 10.0f, 170.0f)) {
            camera->SetFOV(fov);
        }
        if (ImGui::DragFloat("Near Plane", &nearPlane, 0.01f, 0.01f, 100.0f)) {
            camera->nearPlane = nearPlane;
        }
        if (ImGui::DragFloat("Far Plane", &farPlane, 1.0f, 1.0f, 10000.0f)) {
            camera->farPlane = farPlane;
        }
    }
}

void InspectorPanel::DrawMovementComponent(World* world, Entity entity) {
    auto* movement = world->GetComponent<MovementComponent>(entity);
    if (!movement) return;
    
    if (ImGui::CollapsingHeader("Movement")) {
        ImGui::DragFloat("Movement Speed", &movement->movementSpeed, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Mouse Sensitivity", &movement->mouseSensitivity, 0.1f, 0.1f, 10.0f);
        
        ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", 
                   movement->velocity.x, movement->velocity.y, movement->velocity.z);
        ImGui::Text("Pitch: %.2f, Yaw: %.2f", movement->pitch, movement->yaw);
    }
}

}
