#pragma once

#include "../ECS/World.h"
#include "../ECS/Entity.h"
#include "../Components/TransformComponent.h"
#include <memory>
#include <string>
#include <vector>

namespace GameEngine {
    class GameObject {
    public:
        GameObject(World* world, Entity entity);
        GameObject(World* world, Entity entity, const std::string& name);
        ~GameObject() = default;
        
        // Name management
        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }
        
        // Component management
        template<typename T, typename... Args>
        T* AddComponent(Args&&... args);
        
        template<typename T>
        T* GetComponent();
        
        template<typename T>
        const T* GetComponent() const;
        
        template<typename T>
        void RemoveComponent();
        
        template<typename T>
        bool HasComponent() const;
        
        // Transform convenience methods
        TransformComponent* GetTransform();
        const TransformComponent* GetTransform() const;
        
        // Entity access
        Entity GetEntity() const { return m_entity; }
        bool IsValid() const { return m_entity.IsValid(); }
        
        // Hierarchy management
        void SetParent(GameObject* parent);
        GameObject* GetParent() const;
        std::vector<GameObject*> GetChildren() const;
        void AddChild(GameObject* child);
        void RemoveChild(GameObject* child);
        
        // Utility
        void Destroy();
        
    private:
        World* m_world;
        Entity m_entity;
        std::string m_name;
    };
    
    // Template implementations
    template<typename T, typename... Args>
    T* GameObject::AddComponent(Args&&... args) {
        return m_world->AddComponent<T>(m_entity, std::forward<Args>(args)...);
    }
    
    template<typename T>
    T* GameObject::GetComponent() {
        return m_world->GetComponent<T>(m_entity);
    }
    
    template<typename T>
    const T* GameObject::GetComponent() const {
        return m_world->GetComponent<T>(m_entity);
    }
    
    template<typename T>
    void GameObject::RemoveComponent() {
        m_world->RemoveComponent<T>(m_entity);
    }
    
    template<typename T>
    bool GameObject::HasComponent() const {
        return m_world->HasComponent<T>(m_entity);
    }
}
