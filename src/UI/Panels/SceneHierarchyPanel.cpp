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
    
    const auto& entities = world->GetEntities();
    Logger::Debug("Scene Hierarchy panel showing " + std::to_string(entities.size()) + " entities (flat mode - hierarchy display requires Scene reference)");
    
    if (!entities.empty() && !m_selectedEntity.IsValid()) {
        m_selectedEntity = entities[0];
    }
    
    for (const auto& entity : entities) {
        DrawEntityNode(world, entity);
    }
}

void SceneHierarchyPanel::DrawEntityNode(World* world, Entity entity) {
    if (!world || !entity.IsValid()) return;
    
    std::string label = "Entity " + std::to_string(entity.GetID());
    
    Logger::Debug("Drawing entity node: " + label + " (flat mode)");
    
    if (entity.GetID() == 1) {
        m_selectedEntity = entity;
    }
}

}
