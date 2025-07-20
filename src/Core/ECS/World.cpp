#include "World.h"
#include "../Logging/Logger.h"

namespace GameEngine {

World::World() {
    Logger::Info("World created");
}

World::~World() {
    for (auto& system : m_systems) {
        system->Shutdown(this);
    }
    Logger::Info("World destroyed");
}

Entity World::CreateEntity() {
    Entity entity(m_nextEntityID++);
    m_entities.push_back(entity);
    m_components[entity.GetID()] = std::unordered_map<std::type_index, std::unique_ptr<IComponent>>();
    
    Logger::Debug("Entity created with ID: " + std::to_string(entity.GetID()));
    return entity;
}

void World::DestroyEntity(Entity entity) {
    if (!IsEntityValid(entity)) return;
    
    auto it = std::find(m_entities.begin(), m_entities.end(), entity);
    if (it != m_entities.end()) {
        m_entities.erase(it);
    }
    
    m_components.erase(entity.GetID());
    
    Logger::Debug("Entity destroyed with ID: " + std::to_string(entity.GetID()));
}

bool World::IsEntityValid(Entity entity) const {
    return std::find(m_entities.begin(), m_entities.end(), entity) != m_entities.end();
}

void World::Update(float deltaTime) {
    for (auto& system : m_systems) {
        system->Update(this, deltaTime);
    }
}

}
