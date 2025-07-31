#pragma once

#include "GameObject.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace GameEngine {
    class Prefab {
    public:
        Prefab() = default;
        ~Prefab() = default;
        
        // Prefab creation from existing GameObject
        static std::shared_ptr<Prefab> CreateFromGameObject(const GameObject& gameObject);
        
        // GameObject instantiation from Prefab
        GameObject Instantiate(World* world) const;
        GameObject Instantiate(World* world, const Vector3& position) const;
        GameObject Instantiate(World* world, const Vector3& position, const Quaternion& rotation) const;
        
        // Serialization
        bool SaveToFile(const std::string& filepath) const;
        bool LoadFromFile(const std::string& filepath);
        
        // Component data management
        void SetTransformData(const Vector3& position, const Quaternion& rotation, const Vector3& scale);
        void AddComponentData(const std::string& componentType, const std::string& componentData);
        
        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }
        
    private:
        std::string m_name;
        Vector3 m_position = Vector3::Zero;
        Quaternion m_rotation = Quaternion::Identity();
        Vector3 m_scale = Vector3::One;
        std::unordered_map<std::string, std::string> m_componentData;
        
        void SerializeTransform(std::ofstream& file) const;
        void DeserializeTransform(std::ifstream& file);
        void SerializeComponent(std::ofstream& file, const std::string& type, const std::string& data) const;
        std::pair<std::string, std::string> DeserializeComponent(std::ifstream& file) const;
    };
}
