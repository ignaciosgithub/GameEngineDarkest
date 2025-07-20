#include "SceneHierarchyPanel.h"
#include "../../Core/ECS/World.h"
#include "../../Core/Components/TransformComponent.h"
#include "../../Core/Logging/Logger.h"
#include <imgui.h>

namespace GameEngine {

SceneHierarchyPanel::SceneHierarchyPanel() = default;

SceneHierarchyPanel::~SceneHierarchyPanel() = default;

void SceneHierarchyPanel::Update(World* world, float deltaTime) {
    if (!m_visible || !world) return;
    
    ImGui::Begin("Scene Hierarchy", &m_visible);
    
    if (ImGui::Button("Create Entity")) {
        Entity newEntity = world->CreateEntity();
        world->AddComponent<TransformComponent>(newEntity);
        Logger::Info("Created new entity: " + std::to_string(newEntity.GetID()));
    }
    
    ImGui::Separator();
    
    const auto& entities = world->GetEntities();
    for (const auto& entity : entities) {
        DrawEntityNode(world, entity);
    }
    
    if (ImGui::BeginPopupContextWindow()) {
        if (ImGui::MenuItem("Create Empty Entity")) {
            Entity newEntity = world->CreateEntity();
            world->AddComponent<TransformComponent>(newEntity);
            Logger::Info("Created new entity: " + std::to_string(newEntity.GetID()));
        }
        ImGui::EndPopup();
    }
    
    ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(World* world, Entity entity) {
    std::string label = "Entity " + std::to_string(entity.GetID());
    
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (m_selectedEntity == entity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    
    flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    
    bool nodeOpen = ImGui::TreeNodeEx((void*)(uint64_t)entity.GetID(), flags, "%s", label.c_str());
    
    if (ImGui::IsItemClicked()) {
        m_selectedEntity = entity;
    }
    
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Delete Entity")) {
            if (m_selectedEntity == entity) {
                m_selectedEntity = Entity(); // Clear selection
            }
            world->DestroyEntity(entity);
            Logger::Info("Deleted entity: " + std::to_string(entity.GetID()));
        }
        ImGui::EndPopup();
    }
}

}
