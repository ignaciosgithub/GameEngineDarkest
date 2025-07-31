#include "GameObject.h"
#include "../Logging/Logger.h"

namespace GameEngine {
    GameObject::GameObject(World* world, Entity entity) 
        : m_world(world), m_entity(entity), m_name("GameObject_" + std::to_string(entity.GetID())) {
        if (!m_world) {
            Logger::Error("GameObject created with null World pointer");
            return;
        }
        
        if (!m_entity.IsValid()) {
            Logger::Error("GameObject created with invalid Entity");
            return;
        }
        
        if (!HasComponent<TransformComponent>()) {
            AddComponent<TransformComponent>();
            Logger::Debug("Added TransformComponent to GameObject " + std::to_string(m_entity.GetID()));
        }
    }
    
    GameObject::GameObject(World* world, Entity entity, const std::string& name) 
        : m_world(world), m_entity(entity), m_name(name) {
        if (!m_world) {
            Logger::Error("GameObject created with null World pointer");
            return;
        }
        
        if (!m_entity.IsValid()) {
            Logger::Error("GameObject created with invalid Entity");
            return;
        }
        
        if (!HasComponent<TransformComponent>()) {
            AddComponent<TransformComponent>();
            Logger::Debug("Added TransformComponent to GameObject " + std::to_string(m_entity.GetID()));
        }
    }
    
    TransformComponent* GameObject::GetTransform() {
        return GetComponent<TransformComponent>();
    }
    
    const TransformComponent* GameObject::GetTransform() const {
        return GetComponent<TransformComponent>();
    }
    
    void GameObject::Destroy() {
        if (m_world && m_entity.IsValid()) {
            Logger::Debug("Destroying GameObject " + std::to_string(m_entity.GetID()));
            m_world->DestroyEntity(m_entity);
            m_entity = Entity(); // Invalidate the entity
        }
    }
}
