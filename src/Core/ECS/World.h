#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>

namespace GameEngine {
    class World {
    public:
        World();
        ~World();
        
        Entity CreateEntity();
        void DestroyEntity(Entity entity);
        bool IsEntityValid(Entity entity) const;
        
        template<typename T, typename... Args>
        T* AddComponent(Entity entity, Args&&... args);
        
        template<typename T>
        T* GetComponent(Entity entity);
        
        template<typename T>
        const T* GetComponent(Entity entity) const;
        
        template<typename T>
        void RemoveComponent(Entity entity);
        
        template<typename T>
        bool HasComponent(Entity entity) const;
        
        template<typename T, typename... Args>
        void AddSystem(Args&&... args);
        
        template<typename T>
        T* GetSystem();
        
        void Update(float deltaTime);
        
        const std::vector<Entity>& GetEntities() const { return m_entities; }
        
    private:
        EntityID m_nextEntityID = 1;
        std::vector<Entity> m_entities;
        std::unordered_map<EntityID, std::unordered_map<ComponentTypeID, std::unique_ptr<IComponent>>> m_components;
        std::vector<std::unique_ptr<ISystem>> m_systems;
        std::unordered_map<std::type_index, ISystem*> m_systemMap;
    };
    
    // Template implementations
    template<typename T, typename... Args>
    T* World::AddComponent(Entity entity, Args&&... args) {
        if (!IsEntityValid(entity)) return nullptr;
        
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* componentPtr = component.get();
        
        m_components[entity.GetID()][GetComponentTypeID<T>()] = std::move(component);
        return componentPtr;
    }
    
    template<typename T>
    T* World::GetComponent(Entity entity) {
        if (!IsEntityValid(entity)) return nullptr;
        
        auto entityIt = m_components.find(entity.GetID());
        if (entityIt == m_components.end()) return nullptr;
        
        auto componentIt = entityIt->second.find(GetComponentTypeID<T>());
        if (componentIt == entityIt->second.end()) return nullptr;
        
        return static_cast<T*>(componentIt->second.get());
    }
    
    template<typename T>
    const T* World::GetComponent(Entity entity) const {
        return const_cast<World*>(this)->GetComponent<T>(entity);
    }
    
    template<typename T>
    void World::RemoveComponent(Entity entity) {
        if (!IsEntityValid(entity)) return;
        
        auto entityIt = m_components.find(entity.GetID());
        if (entityIt != m_components.end()) {
            entityIt->second.erase(GetComponentTypeID<T>());
        }
    }
    
    template<typename T>
    bool World::HasComponent(Entity entity) const {
        if (!IsEntityValid(entity)) return false;
        
        auto entityIt = m_components.find(entity.GetID());
        if (entityIt == m_components.end()) return false;
        
        return entityIt->second.find(GetComponentTypeID<T>()) != entityIt->second.end();
    }
    
    template<typename T, typename... Args>
    void World::AddSystem(Args&&... args) {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* systemPtr = system.get();
        
        m_systemMap[std::type_index(typeid(T))] = systemPtr;
        m_systems.push_back(std::move(system));
        
        systemPtr->Initialize(this);
    }
    
    template<typename T>
    T* World::GetSystem() {
        auto it = m_systemMap.find(std::type_index(typeid(T)));
        if (it != m_systemMap.end()) {
            return static_cast<T*>(it->second);
        }
        return nullptr;
    }
}
