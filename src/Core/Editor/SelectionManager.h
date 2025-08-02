#pragma once

#include "../ECS/Entity.h"
#include "../ECS/World.h"

namespace GameEngine {
    class SelectionManager {
    public:
        SelectionManager();
        ~SelectionManager();
        
        void SetSelectedEntity(Entity entity);
        void ClearSelection();
        
        Entity GetSelectedEntity() const { return m_selectedEntity; }
        bool HasSelection() const { return m_selectedEntity.IsValid(); }
        bool IsSelected(Entity entity) const { return m_selectedEntity == entity; }
        
        void Update(World* world);
        
    private:
        Entity m_selectedEntity;
    };
}
