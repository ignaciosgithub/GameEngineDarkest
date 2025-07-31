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
    
    void GameObject::SetParent(GameObject* parent) {
        if (!IsValid()) {
            Logger::Warning("Attempted to set parent on invalid GameObject");
            return;
        }
        
        auto* transform = GetTransform();
        if (!transform) {
            Logger::Error("GameObject has no TransformComponent for hierarchy");
            return;
        }
        
        if (parent && parent->IsValid()) {
            auto* parentTransform = parent->GetTransform();
            if (parentTransform) {
                transform->transform.SetParent(&parentTransform->transform);
                Logger::Debug("Set parent for GameObject " + std::to_string(m_entity.GetID()) + 
                             " to GameObject " + std::to_string(parent->GetEntity().GetID()));
            }
        } else {
            transform->transform.SetParent(nullptr);
            Logger::Debug("Removed parent from GameObject " + std::to_string(m_entity.GetID()));
        }
    }
    
    GameObject* GameObject::GetParent() const {
        if (!IsValid()) {
            Logger::Warning("GetParent called on invalid GameObject");
            return nullptr;
        }
        
        auto* transform = GetTransform();
        if (!transform) {
            Logger::Error("GameObject has no TransformComponent for hierarchy query");
            return nullptr;
        }
        
        Transform* parentTransform = transform->transform.GetParent();
        if (!parentTransform) {
            return nullptr; // No parent
        }
        
        Logger::Debug("GetParent requires Scene context - returning nullptr (Transform parent exists but GameObject lookup unavailable)");
        return nullptr;
    }
    
    std::vector<GameObject*> GameObject::GetChildren() const {
        if (!IsValid()) {
            Logger::Warning("GetChildren called on invalid GameObject");
            return {};
        }
        
        auto* transform = GetTransform();
        if (!transform) {
            Logger::Error("GameObject has no TransformComponent for hierarchy query");
            return {};
        }
        
        Logger::Debug("GetChildren requires Scene context - returning empty vector (use Scene::FindChildrenOf instead)");
        return {};
    }
    
    void GameObject::AddChild(GameObject* child) {
        if (child && child->IsValid()) {
            child->SetParent(this);
        }
    }
    
    void GameObject::RemoveChild(GameObject* child) {
        if (child && child->IsValid() && child->GetParent() == this) {
            child->SetParent(nullptr);
        }
    }
}
