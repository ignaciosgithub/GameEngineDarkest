#pragma once

#include "../GameObject/GameObject.h"
#include "../GameObject/Prefab.h"
#include <vector>
#include <memory>
#include <string>

namespace GameEngine {
    class Scene {
    public:
        Scene(World* world, const std::string& name = "Untitled Scene");
        ~Scene();
        
        // GameObject management
        GameObject CreateGameObject(const std::string& name = "GameObject");
        GameObject CreateGameObject(const Vector3& position, const std::string& name = "GameObject");
        void DestroyGameObject(const GameObject& gameObject);
        
        // Prefab instantiation
        GameObject InstantiatePrefab(std::shared_ptr<Prefab> prefab);
        GameObject InstantiatePrefab(std::shared_ptr<Prefab> prefab, const Vector3& position);
        
        // Scene management
        void Clear();
        const std::vector<GameObject>& GetGameObjects() const { return m_gameObjects; }
        size_t GetGameObjectCount() const { return m_gameObjects.size(); }
        
        // Serialization
        bool SaveToFile(const std::string& filepath) const;
        bool LoadFromFile(const std::string& filepath);
        
        // Properties
        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }
        
        World* GetWorld() const { return m_world; }
        
        // GameObject search
        GameObject* FindGameObjectByName(const std::string& name);
        std::vector<GameObject*> FindGameObjectsByName(const std::string& name);
        
    private:
        World* m_world;
        std::string m_name;
        std::vector<GameObject> m_gameObjects;
        
        void RegisterGameObject(const GameObject& gameObject);
        void UnregisterGameObject(const GameObject& gameObject);
        
        // Serialization helpers
        void SerializeGameObject(std::ofstream& file, const GameObject& gameObject) const;
        GameObject DeserializeGameObject(std::ifstream& file);
    };
}
