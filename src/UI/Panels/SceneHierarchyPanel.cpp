#include "SceneHierarchyPanel.h"
#include "../../Core/ECS/World.h"
#include "../../Core/Components/TransformComponent.h"
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
            if (ImGui::MenuItem("Create Empty Entity")) {
                Entity newEntity = world->CreateEntity();
                world->AddComponent<TransformComponent>(newEntity);
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
    
    if (opened) {
        ImGui::TreePop();
    }
}

}
