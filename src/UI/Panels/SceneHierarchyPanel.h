#pragma once

#include "UIPanel.h"
#include "../../Core/ECS/Entity.h"

namespace GameEngine {
    class SceneHierarchyPanel : public UIPanel {
    public:
        SceneHierarchyPanel();
        ~SceneHierarchyPanel() override;
        
        void Update(World* world, float deltaTime) override;
        
        Entity GetSelectedEntity() const { return m_selectedEntity; }
        void SetSelectedEntity(Entity entity) { m_selectedEntity = entity; }
        
    private:
        void DrawEntityNode(World* world, Entity entity);
        
        Entity m_selectedEntity;
    };
}
