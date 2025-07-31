#include "Prefab.h"
#include "../Logging/Logger.h"
#include "../Components/MovementComponent.h"
#include "../Components/CameraComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../../Rendering/Lighting/Light.h"
#include <fstream>
#include <sstream>

namespace GameEngine {
    std::shared_ptr<Prefab> Prefab::CreateFromGameObject(const GameObject& gameObject) {
        if (!gameObject.IsValid()) {
            Logger::Error("Cannot create Prefab from invalid GameObject");
            return nullptr;
        }
        
        auto prefab = std::make_shared<Prefab>();
        prefab->SetName("GameObject_" + std::to_string(gameObject.GetEntity().GetID()));
        
        if (auto* transform = gameObject.GetTransform()) {
            prefab->SetTransformData(
                transform->transform.GetPosition(),
                transform->transform.GetRotation(),
                transform->transform.GetScale()
            );
        }
        
        if (gameObject.HasComponent<MovementComponent>()) {
            auto* movement = gameObject.GetComponent<MovementComponent>();
            std::stringstream ss;
            ss << movement->movementSpeed << "," << movement->mouseSensitivity;
            prefab->AddComponentData("MovementComponent", ss.str());
        }
        
        if (gameObject.HasComponent<CameraComponent>()) {
            auto* camera = gameObject.GetComponent<CameraComponent>();
            std::stringstream ss;
            ss << camera->fieldOfView << "," << camera->nearPlane << "," << camera->farPlane;
            prefab->AddComponentData("CameraComponent", ss.str());
        }
        
        if (gameObject.HasComponent<LightComponent>()) {
            auto* light = gameObject.GetComponent<LightComponent>();
            std::stringstream ss;
            ss << static_cast<int>(light->light.GetType()) << "," 
               << light->light.GetIntensity() << ","
               << light->light.GetColor().x << "," << light->light.GetColor().y << "," << light->light.GetColor().z;
            prefab->AddComponentData("LightComponent", ss.str());
        }
        
        Logger::Info("Created Prefab from GameObject " + std::to_string(gameObject.GetEntity().GetID()));
        return prefab;
    }
    
    GameObject Prefab::Instantiate(World* world) const {
        return Instantiate(world, m_position, m_rotation);
    }
    
    GameObject Prefab::Instantiate(World* world, const Vector3& position) const {
        return Instantiate(world, position, m_rotation);
    }
    
    GameObject Prefab::Instantiate(World* world, const Vector3& position, const Quaternion& rotation) const {
        if (!world) {
            Logger::Error("Cannot instantiate Prefab with null World");
            return GameObject(nullptr, Entity());
        }
        
        Entity entity = world->CreateEntity();
        GameObject gameObject(world, entity);
        
        if (auto* transform = gameObject.GetTransform()) {
            transform->transform.SetPosition(position);
            transform->transform.SetRotation(rotation);
            transform->transform.SetScale(m_scale);
        }
        
        for (const auto& [componentType, componentData] : m_componentData) {
            if (componentType == "MovementComponent") {
                std::stringstream ss(componentData);
                std::string speedStr, sensitivityStr;
                std::getline(ss, speedStr, ',');
                std::getline(ss, sensitivityStr, ',');
                
                float speed = std::stof(speedStr);
                float sensitivity = std::stof(sensitivityStr);
                gameObject.AddComponent<MovementComponent>(speed, sensitivity);
            }
            else if (componentType == "CameraComponent") {
                std::stringstream ss(componentData);
                std::string fovStr, nearStr, farStr;
                std::getline(ss, fovStr, ',');
                std::getline(ss, nearStr, ',');
                std::getline(ss, farStr, ',');
                
                auto* camera = gameObject.AddComponent<CameraComponent>();
                camera->fieldOfView = std::stof(fovStr);
                camera->nearPlane = std::stof(nearStr);
                camera->farPlane = std::stof(farStr);
            }
            else if (componentType == "LightComponent") {
                std::stringstream ss(componentData);
                std::string typeStr, intensityStr, rStr, gStr, bStr;
                std::getline(ss, typeStr, ',');
                std::getline(ss, intensityStr, ',');
                std::getline(ss, rStr, ',');
                std::getline(ss, gStr, ',');
                std::getline(ss, bStr, ',');
                
                LightType type = static_cast<LightType>(std::stoi(typeStr));
                float intensity = std::stof(intensityStr);
                Vector3 color(std::stof(rStr), std::stof(gStr), std::stof(bStr));
                
                auto* light = gameObject.AddComponent<LightComponent>(type);
                light->light.SetIntensity(intensity);
                light->light.SetColor(color);
            }
        }
        
        Logger::Info("Instantiated GameObject from Prefab: " + m_name);
        return gameObject;
    }
    
    bool Prefab::SaveToFile(const std::string& filepath) const {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            Logger::Error("Failed to open file for writing: " + filepath);
            return false;
        }
        
        file << "# GameEngine Prefab File\n";
        file << "Name: " << m_name << "\n";
        file << "\n";
        
        SerializeTransform(file);
        
        file << "\n[Components]\n";
        for (const auto& [type, data] : m_componentData) {
            SerializeComponent(file, type, data);
        }
        
        file.close();
        Logger::Info("Saved Prefab to file: " + filepath);
        return true;
    }
    
    bool Prefab::LoadFromFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            Logger::Error("Failed to open file for reading: " + filepath);
            return false;
        }
        
        std::string line;
        bool inComponents = false;
        
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            if (line.find("Name: ") == 0) {
                m_name = line.substr(6);
            }
            else if (line == "[Transform]") {
                DeserializeTransform(file);
            }
            else if (line == "[Components]") {
                inComponents = true;
            }
            else if (inComponents && line.find("Component: ") == 0) {
                auto [type, data] = DeserializeComponent(file);
                if (!type.empty()) {
                    AddComponentData(type, data);
                }
            }
        }
        
        file.close();
        Logger::Info("Loaded Prefab from file: " + filepath);
        return true;
    }
    
    void Prefab::SetTransformData(const Vector3& position, const Quaternion& rotation, const Vector3& scale) {
        m_position = position;
        m_rotation = rotation;
        m_scale = scale;
    }
    
    void Prefab::AddComponentData(const std::string& componentType, const std::string& componentData) {
        m_componentData[componentType] = componentData;
    }
    
    void Prefab::SerializeTransform(std::ofstream& file) const {
        file << "[Transform]\n";
        file << "Position: " << m_position.x << "," << m_position.y << "," << m_position.z << "\n";
        file << "Rotation: " << m_rotation.x << "," << m_rotation.y << "," << m_rotation.z << "," << m_rotation.w << "\n";
        file << "Scale: " << m_scale.x << "," << m_scale.y << "," << m_scale.z << "\n";
    }
    
    void Prefab::DeserializeTransform(std::ifstream& file) {
        std::string line;
        while (std::getline(file, line) && !line.empty() && line[0] != '[') {
            if (line.find("Position: ") == 0) {
                std::stringstream ss(line.substr(10));
                std::string x, y, z;
                std::getline(ss, x, ',');
                std::getline(ss, y, ',');
                std::getline(ss, z, ',');
                m_position = Vector3(std::stof(x), std::stof(y), std::stof(z));
            }
            else if (line.find("Rotation: ") == 0) {
                std::stringstream ss(line.substr(10));
                std::string x, y, z, w;
                std::getline(ss, x, ',');
                std::getline(ss, y, ',');
                std::getline(ss, z, ',');
                std::getline(ss, w, ',');
                m_rotation = Quaternion(std::stof(x), std::stof(y), std::stof(z), std::stof(w));
            }
            else if (line.find("Scale: ") == 0) {
                std::stringstream ss(line.substr(7));
                std::string x, y, z;
                std::getline(ss, x, ',');
                std::getline(ss, y, ',');
                std::getline(ss, z, ',');
                m_scale = Vector3(std::stof(x), std::stof(y), std::stof(z));
            }
        }
    }
    
    void Prefab::SerializeComponent(std::ofstream& file, const std::string& type, const std::string& data) const {
        file << "Component: " << type << "\n";
        file << "Data: " << data << "\n";
    }
    
    std::pair<std::string, std::string> Prefab::DeserializeComponent(std::ifstream& file) const {
        std::string type, data;
        std::string line;
        
        while (std::getline(file, line) && !line.empty() && line[0] != '[' && line.find("Component: ") != 0) {
            if (line.find("Data: ") == 0) {
                data = line.substr(6);
                break;
            }
        }
        
        return {type, data};
    }
}
