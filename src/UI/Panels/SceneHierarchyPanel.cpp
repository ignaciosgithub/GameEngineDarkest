#include "SceneHierarchyPanel.h"
#include "../../Core/ECS/World.h"
#include "../../Core/Components/TransformComponent.h"
#include "../../Core/Components/MeshComponent.h"
#include "../../Rendering/Lighting/Light.h"
#include "../../Core/Logging/Logger.h"
#include <imgui.h>

namespace GameEngine {

SceneHierarchyPanel::SceneHierarchyPanel() = default;

SceneHierarchyPanel::~SceneHierarchyPanel() = default;

void SceneHierarchyPanel::Update(World* world, float /*deltaTime*/) {
    if (!m_visible || !world) return;
    
    if (ImGui::Begin("Scene Hierarchy", &m_visible)) {
        const auto& entities = world->GetEntities();
        
        for (const auto& entity : entities) {
            DrawEntityNode(world, entity);
        }
        
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Create Empty GameObject")) {
                Entity newEntity = world->CreateEntity();
                world->AddComponent<TransformComponent>(newEntity);
                Logger::Info("Created Empty GameObject with Entity ID: " + std::to_string(newEntity.GetID()));
            }
            if (ImGui::MenuItem("Create Cube")) {
                Entity newEntity = world->CreateEntity();
                world->AddComponent<TransformComponent>(newEntity);
                auto* meshComp = world->AddComponent<MeshComponent>(newEntity, "cube");
                if (meshComp) {
                    meshComp->SetColor(Vector3(0.8f, 0.8f, 0.8f));
                }
                Logger::Info("Created Cube GameObject with Entity ID: " + std::to_string(newEntity.GetID()));
            }
            if (ImGui::MenuItem("Create Light")) {
                Entity newEntity = world->CreateEntity();
                world->AddComponent<TransformComponent>(newEntity);
                world->AddComponent<LightComponent>(newEntity, LightType::Point);
                Logger::Info("Created Light GameObject with Entity ID: " + std::to_string(newEntity.GetID()));
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(World* world, Entity entity) {
    if (!world || !entity.IsValid()) return;
    
    std::string label = "Entity " + std::to_string(entity.GetID());
    
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (m_selectedEntity == entity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    
    bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetID(), flags, "%s", label.c_str());
    
    if (ImGui::IsItemClicked()) {
        m_selectedEntity = entity;
    }
    
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        m_selectedEntity = entity;
        ImGui::OpenPopup("EntityContextMenu");
    }
    
    if (ImGui::BeginPopup("EntityContextMenu")) {
        if (ImGui::MenuItem("Delete Entity")) {
            world->DestroyEntity(m_selectedEntity);
            m_selectedEntity = Entity(); // Clear selection
            Logger::Info("Deleted Entity ID: " + std::to_string(entity.GetID()));
        }
        ImGui::EndPopup();
    }
    
    if (opened) {
        ImGui::TreePop();
    }
}

}
