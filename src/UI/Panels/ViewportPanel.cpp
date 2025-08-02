#include "ViewportPanel.h"
#include "../../Core/Logging/Logger.h"
#include "../../Core/Editor/PlayModeManager.h"
#include "../../Core/Editor/SelectionManager.h"
#include "../../Core/Physics/RayCaster.h"
#include "../../Core/Systems/CameraSystem.h"
#include "../../Core/Components/CameraComponent.h"
#include "../../Core/Components/TransformComponent.h"
#include "../../Core/Components/MeshComponent.h"
#include <imgui.h>

namespace GameEngine {

ViewportPanel::ViewportPanel() = default;

ViewportPanel::~ViewportPanel() = default;

void ViewportPanel::Update(World* world, float /*deltaTime*/) {
    if (!m_visible) return;
    
    if (ImGui::Begin("Viewport", &m_visible)) {
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        
        if (viewportPanelSize.x != m_viewportSize.x || viewportPanelSize.y != m_viewportSize.y) {
            m_viewportSize = viewportPanelSize;
            m_viewportResized = true;
        }
        
        if (m_sceneFramebuffer) {
            auto colorTexture = m_sceneFramebuffer->GetColorTexture(0);
            if (colorTexture) {
                ImGui::Image((ImTextureID)(intptr_t)colorTexture->GetID(), 
                            ImVec2(m_viewportSize.x, m_viewportSize.y), 
                            ImVec2(0, 1), ImVec2(1, 0));
                
                if (ImGui::IsItemClicked() && m_playModeManager && m_playModeManager->GetCurrentMode() == EditorMode::Edit) {
                    ImVec2 mousePos = ImGui::GetMousePos();
                    ImVec2 itemMin = ImGui::GetItemRectMin();
                    Vector2 relativePos(mousePos.x - itemMin.x, mousePos.y - itemMin.y);
                    HandleViewportClick(relativePos, world);
                }
            } else {
                ImGui::Text("Color texture not available");
            }
        } else {
            ImGui::Text("Scene rendering not available");
            ImGui::Text("Viewport Size: %.0f x %.0f", m_viewportSize.x, m_viewportSize.y);
        }
        
        m_viewportFocused = ImGui::IsWindowFocused();
        m_viewportHovered = ImGui::IsWindowHovered();
    }
    ImGui::End();
}

void ViewportPanel::HandleViewportClick(const Vector2& relativePos, World* world) {
    Logger::Debug("Viewport clicked at relative position: (" + std::to_string(relativePos.x) + ", " + std::to_string(relativePos.y) + ")");
    
    if (!world || !m_selectionManager) {
        Logger::Warning("HandleViewportClick: Missing world or selection manager");
        return;
    }
    
    auto* cameraSystem = world->GetSystem<CameraSystem>();
    if (!cameraSystem) {
        Logger::Warning("HandleViewportClick: No CameraSystem found");
        return;
    }
    
    Entity activeCamera = cameraSystem->GetActiveCamera();
    if (!activeCamera.IsValid()) {
        Logger::Warning("HandleViewportClick: No active camera found");
        return;
    }
    
    auto* cameraComponent = world->GetComponent<CameraComponent>(activeCamera);
    auto* cameraTransform = world->GetComponent<TransformComponent>(activeCamera);
    if (!cameraComponent || !cameraTransform) {
        Logger::Warning("HandleViewportClick: Active camera missing required components");
        return;
    }
    
    RayCaster rayCaster;
    int screenWidth = static_cast<int>(m_viewportSize.x);
    int screenHeight = static_cast<int>(m_viewportSize.y);
    
    Ray3D ray = rayCaster.ScreenPointToRay(relativePos, cameraComponent, cameraTransform->transform, screenWidth, screenHeight);
    
    Entity closestEntity;
    float closestDistance = FLT_MAX;
    
    const auto& entities = world->GetEntities();
    for (const Entity& entity : entities) {
        if (!world->HasComponent<MeshComponent>(entity) || !world->HasComponent<TransformComponent>(entity)) {
            continue;
        }
        
        auto* meshComponent = world->GetComponent<MeshComponent>(entity);
        auto* transformComponent = world->GetComponent<TransformComponent>(entity);
        
        if (!meshComponent->IsVisible()) {
            continue;
        }
        
        Vector3 entityPos = transformComponent->transform.GetPosition();
        float boundingRadius = 1.0f; // Simple sphere approximation for now
        
        Vector3 rayToEntity = entityPos - ray.origin;
        float projectionLength = rayToEntity.Dot(ray.direction);
        
        if (projectionLength < 0) continue; // Entity is behind ray origin
        
        Vector3 closestPointOnRay = ray.origin + ray.direction * projectionLength;
        float distanceToEntity = (entityPos - closestPointOnRay).Length();
        
        if (distanceToEntity <= boundingRadius && projectionLength < closestDistance) {
            closestEntity = entity;
            closestDistance = projectionLength;
        }
    }
    
    if (closestEntity.IsValid()) {
        m_selectionManager->SetSelectedEntity(closestEntity);
        Logger::Info("Selected entity: " + std::to_string(closestEntity.GetID()));
    } else {
        m_selectionManager->ClearSelection();
        Logger::Info("No entity selected - cleared selection");
    }
}

}
