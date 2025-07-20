#pragma once

#include "UIPanel.h"
#include "../../Core/ECS/Entity.h"

namespace GameEngine {
    class InspectorPanel : public UIPanel {
    public:
        InspectorPanel();
        ~InspectorPanel() override;
        
        void Update(World* world, float deltaTime) override;
        
        void SetSelectedEntity(Entity entity) { m_selectedEntity = entity; }
        
    private:
        void DrawTransformComponent(World* world, Entity entity);
        void DrawCameraComponent(World* world, Entity entity);
        void DrawMovementComponent(World* world, Entity entity);
        
        Entity m_selectedEntity;
    };
}
