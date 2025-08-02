#include "SelectionManager.h"
#include "../Logging/Logger.h"

namespace GameEngine {

SelectionManager::SelectionManager() {
    Logger::Info("SelectionManager initialized");
}

SelectionManager::~SelectionManager() {
    Logger::Info("SelectionManager destroyed");
}

void SelectionManager::SetSelectedEntity(Entity entity) {
    if (m_selectedEntity != entity) {
        Entity previousSelection = m_selectedEntity;
        m_selectedEntity = entity;
        
        if (previousSelection.IsValid()) {
            Logger::Debug("Deselected entity: " + std::to_string(previousSelection.GetID()));
        }
        
        if (entity.IsValid()) {
            Logger::Debug("Selected entity: " + std::to_string(entity.GetID()));
        }
    }
}

void SelectionManager::ClearSelection() {
    if (m_selectedEntity.IsValid()) {
        Logger::Debug("Cleared selection of entity: " + std::to_string(m_selectedEntity.GetID()));
        m_selectedEntity = Entity();
    }
}

void SelectionManager::Update(World* world) {
    if (m_selectedEntity.IsValid() && world) {
        if (!world->IsEntityValid(m_selectedEntity)) {
            Logger::Warning("Selected entity " + std::to_string(m_selectedEntity.GetID()) + " is no longer valid, clearing selection");
            ClearSelection();
        }
    }
}

}
