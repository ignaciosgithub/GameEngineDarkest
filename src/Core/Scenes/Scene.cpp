#include "Scene.h"
#include "../Logging/Logger.h"
#include "../Components/MovementComponent.h"
#include "../Components/CameraComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/MeshComponent.h"
#include "../../Rendering/Lighting/Light.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace GameEngine {
    Scene::Scene(World* world, const std::string& name) 
        : m_world(world), m_name(name) {
        if (!m_world) {
            Logger::Error("Scene created with null World pointer");
            return;
        }
        
        Logger::Info("Created Scene: " + m_name);
    }
    
    Scene::~Scene() {
        Clear();
        Logger::Info("Destroyed Scene: " + m_name);
    }
    
    GameObject Scene::CreateGameObject(const std::string& name) {
        return CreateGameObject(Vector3::Zero, name);
    }
    
    GameObject Scene::CreateGameObject(const Vector3& position, const std::string& name) {
        if (!m_world) {
            Logger::Error("Cannot create GameObject in Scene with null World");
            return GameObject(nullptr, Entity());
        }
        
        Entity entity = m_world->CreateEntity();
        GameObject gameObject(m_world, entity, name);
        
        if (auto* transform = gameObject.GetTransform()) {
            transform->transform.SetPosition(position);
        }
        
        RegisterGameObject(gameObject);
        
        Logger::Debug("Created GameObject '" + name + "' in Scene '" + m_name + "' at position (" + 
                     std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + ")");
        
        return gameObject;
    }
    
    void Scene::DestroyGameObject(const GameObject& gameObject) {
        if (!gameObject.IsValid()) {
            Logger::Warning("Attempted to destroy invalid GameObject");
            return;
        }
        
        UnregisterGameObject(gameObject);
        
        Logger::Debug("Destroyed GameObject " + std::to_string(gameObject.GetEntity().GetID()) + " from Scene '" + m_name + "'");
    }
    
    GameObject Scene::InstantiatePrefab(std::shared_ptr<Prefab> prefab) {
        if (!prefab) {
            Logger::Error("Cannot instantiate null Prefab");
            return GameObject(nullptr, Entity());
        }
        
        GameObject gameObject = prefab->Instantiate(m_world);
        if (gameObject.IsValid()) {
            RegisterGameObject(gameObject);
            Logger::Info("Instantiated Prefab '" + prefab->GetName() + "' in Scene '" + m_name + "'");
        }
        
        return gameObject;
    }
    
    GameObject Scene::InstantiatePrefab(std::shared_ptr<Prefab> prefab, const Vector3& position) {
        if (!prefab) {
            Logger::Error("Cannot instantiate null Prefab");
            return GameObject(nullptr, Entity());
        }
        
        GameObject gameObject = prefab->Instantiate(m_world, position);
        if (gameObject.IsValid()) {
            RegisterGameObject(gameObject);
            Logger::Info("Instantiated Prefab '" + prefab->GetName() + "' at position (" + 
                        std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + 
                        ") in Scene '" + m_name + "'");
        }
        
        return gameObject;
    }
    
    void Scene::Clear() {
        Logger::Info("Clearing Scene '" + m_name + "' with " + std::to_string(m_gameObjects.size()) + " GameObjects");
        
        for (auto& gameObject : m_gameObjects) {
            if (gameObject.IsValid()) {
                gameObject.Destroy();
            }
        }
        
        m_gameObjects.clear();
    }
    
    bool Scene::SaveToFile(const std::string& filepath) const {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            Logger::Error("Failed to open file for writing: " + filepath);
            return false;
        }
        
        file << "# GameEngine Scene File\n";
        file << "Name: " << m_name << "\n";
        file << "GameObjectCount: " << m_gameObjects.size() << "\n";
        file << "\n";
        
        for (size_t i = 0; i < m_gameObjects.size(); ++i) {
            file << "[GameObject_" << i << "]\n";
            SerializeGameObject(file, m_gameObjects[i]);
            file << "\n";
        }
        
        file.close();
        Logger::Info("Saved Scene '" + m_name + "' to file: " + filepath);
        return true;
    }
    
    bool Scene::LoadFromFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            Logger::Error("Failed to open file for reading: " + filepath);
            return false;
        }
        
        Clear();
        
        std::string line;
        size_t expectedCount = 0;
        std::vector<std::pair<uint32_t, uint32_t>> parentChildPairs; // child ID, parent ID
        
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            if (line.find("Name: ") == 0) {
                m_name = line.substr(6);
            }
            else if (line.find("GameObjectCount: ") == 0) {
                expectedCount = std::stoul(line.substr(17));
            }
            else if (line.find("[GameObject_") == 0) {
                std::streampos currentPos = file.tellg();
                
                GameObject gameObject = DeserializeGameObject(file);
                if (gameObject.IsValid()) {
                    RegisterGameObject(gameObject);
                    
                    std::streampos endPos = file.tellg();
                    file.seekg(currentPos);
                    
                    std::string objLine;
                    while (std::getline(file, objLine) && !objLine.empty() && objLine[0] != '[') {
                        if (objLine.find("ParentID: ") == 0) {
                            uint32_t parentID = std::stoul(objLine.substr(10));
                            parentChildPairs.emplace_back(gameObject.GetEntity().GetID(), parentID);
                            break;
                        }
                    }
                    
                    file.seekg(endPos);
                }
            }
        }
        
        file.close();
        
        for (const auto& pair : parentChildPairs) {
            uint32_t childID = pair.first;
            uint32_t parentID = pair.second;
            
            GameObject* child = nullptr;
            GameObject* parent = nullptr;
            
            for (auto& gameObject : m_gameObjects) {
                if (gameObject.GetEntity().GetID() == childID) {
                    child = &gameObject;
                }
                if (gameObject.GetEntity().GetID() == parentID) {
                    parent = &gameObject;
                }
            }
            
            if (child && parent) {
                child->SetParent(parent);
                Logger::Debug("Restored parent-child relationship: Child " + std::to_string(childID) + 
                             " -> Parent " + std::to_string(parentID));
            } else {
                Logger::Warning("Failed to restore parent-child relationship: Child " + std::to_string(childID) + 
                               " -> Parent " + std::to_string(parentID) + " (GameObject not found)");
            }
        }
        
        if (m_gameObjects.size() != expectedCount) {
            Logger::Warning("Scene loaded with " + std::to_string(m_gameObjects.size()) + 
                           " GameObjects, expected " + std::to_string(expectedCount));
        }
        
        Logger::Info("Loaded Scene '" + m_name + "' from file: " + filepath + 
                    " with " + std::to_string(parentChildPairs.size()) + " parent-child relationships restored");
        return true;
    }
    
    GameObject* Scene::FindGameObjectByName(const std::string& name) {
        for (auto& gameObject : m_gameObjects) {
            if (gameObject.IsValid() && gameObject.GetName() == name) {
                return &gameObject;
            }
        }
        return nullptr;
    }
    
    std::vector<GameObject*> Scene::FindGameObjectsByName(const std::string& name) {
        std::vector<GameObject*> results;
        for (auto& gameObject : m_gameObjects) {
            if (gameObject.IsValid() && gameObject.GetName() == name) {
                results.push_back(&gameObject);
            }
        }
        return results;
    }
    
    GameObject* Scene::FindGameObjectByTransform(const Transform* transform) const {
        if (!transform) return nullptr;
        
        for (auto& gameObject : m_gameObjects) {
            if (gameObject.IsValid()) {
                auto* objTransform = gameObject.GetTransform();
                if (objTransform && &objTransform->transform == transform) {
                    return const_cast<GameObject*>(&gameObject);
                }
            }
        }
        return nullptr;
    }
    
    std::vector<GameObject*> Scene::FindChildrenOf(const GameObject* parent) const {
        std::vector<GameObject*> children;
        if (!parent || !parent->IsValid()) return children;
        
        auto* parentTransform = parent->GetTransform();
        if (!parentTransform) return children;
        
        for (auto& gameObject : m_gameObjects) {
            if (gameObject.IsValid() && &gameObject != parent) {
                auto* transform = gameObject.GetTransform();
                if (transform && transform->transform.GetParent() == &parentTransform->transform) {
                    children.push_back(const_cast<GameObject*>(&gameObject));
                }
            }
        }
        return children;
    }
    
    std::vector<GameObject*> Scene::GetRootGameObjects() const {
        std::vector<GameObject*> roots;
        for (auto& gameObject : m_gameObjects) {
            if (gameObject.IsValid()) {
                auto* transform = gameObject.GetTransform();
                if (transform && !transform->transform.GetParent()) {
                    roots.push_back(const_cast<GameObject*>(&gameObject));
                }
            }
        }
        return roots;
    }
    
    void Scene::RegisterGameObject(const GameObject& gameObject) {
        m_gameObjects.push_back(gameObject);
    }
    
    void Scene::UnregisterGameObject(const GameObject& gameObject) {
        auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
            [&gameObject](const GameObject& obj) {
                return obj.GetEntity().GetID() == gameObject.GetEntity().GetID();
            });
        
        if (it != m_gameObjects.end()) {
            m_gameObjects.erase(it);
        }
    }
    
    void Scene::SerializeGameObject(std::ofstream& file, const GameObject& gameObject) const {
        if (!gameObject.IsValid()) return;
        
        file << "EntityID: " << gameObject.GetEntity().GetID() << "\n";
        
        if (auto* transform = gameObject.GetTransform()) {
            file << "Transform: " << transform->transform.GetPosition().x << "," 
                 << transform->transform.GetPosition().y << "," << transform->transform.GetPosition().z << ","
                 << transform->transform.GetRotation().x << "," << transform->transform.GetRotation().y << ","
                 << transform->transform.GetRotation().z << "," << transform->transform.GetRotation().w << ","
                 << transform->transform.GetScale().x << "," << transform->transform.GetScale().y << ","
                 << transform->transform.GetScale().z << "\n";
        }
        
        if (gameObject.HasComponent<MovementComponent>()) {
            auto* movement = gameObject.GetComponent<MovementComponent>();
            file << "MovementComponent: " << movement->movementSpeed << "," << movement->mouseSensitivity << "\n";
        }
        
        if (gameObject.HasComponent<CameraComponent>()) {
            auto* camera = gameObject.GetComponent<CameraComponent>();
            file << "CameraComponent: " << camera->fieldOfView << "," << camera->nearPlane << "," << camera->farPlane << "\n";
        }
        
        if (gameObject.HasComponent<LightComponent>()) {
            auto* light = gameObject.GetComponent<LightComponent>();
            file << "LightComponent: " << static_cast<int>(light->light.GetType()) << "," 
                 << light->light.GetIntensity() << ","
                 << light->light.GetColor().x << "," << light->light.GetColor().y << "," << light->light.GetColor().z << "\n";
        }

        if (gameObject.HasComponent<MeshComponent>()) {
            auto* mesh = gameObject.GetComponent<MeshComponent>();
            file << "MeshComponent: " << mesh->GetMeshType() << ","
                 << mesh->GetColor().x << "," << mesh->GetColor().y << "," << mesh->GetColor().z << ","
                 << mesh->GetMetallic() << "," << mesh->GetRoughness() << ","
                 << (mesh->IsVisible() ? 1 : 0) << "\n";
        }
        
        if (auto* transform = gameObject.GetTransform()) {
            if (transform->transform.GetParent()) {
                GameObject* parent = FindGameObjectByTransform(transform->transform.GetParent());
                if (parent) {
                    file << "ParentID: " << parent->GetEntity().GetID() << "\n";
                }
            }
        }
    }
    
    GameObject Scene::DeserializeGameObject(std::ifstream& file) {
        if (!m_world) {
            Logger::Error("Cannot deserialize GameObject with null World");
            return GameObject(nullptr, Entity());
        }
        
        Entity entity = m_world->CreateEntity();
        GameObject gameObject(m_world, entity);
        
        std::string line;
        while (std::getline(file, line) && !line.empty() && line[0] != '[') {
            if (line.find("Transform: ") == 0) {
                std::stringstream ss(line.substr(11));
                std::string values[10];
                for (int i = 0; i < 10; ++i) {
                    std::getline(ss, values[i], ',');
                }
                
                if (auto* transform = gameObject.GetTransform()) {
                    Vector3 position(std::stof(values[0]), std::stof(values[1]), std::stof(values[2]));
                    Quaternion rotation(std::stof(values[3]), std::stof(values[4]), std::stof(values[5]), std::stof(values[6]));
                    Vector3 scale(std::stof(values[7]), std::stof(values[8]), std::stof(values[9]));
                    
                    transform->transform.SetPosition(position);
                    transform->transform.SetRotation(rotation);
                    transform->transform.SetScale(scale);
                }
            }
            else if (line.find("MovementComponent: ") == 0) {
                std::stringstream ss(line.substr(19));
                std::string speedStr, sensitivityStr;
                std::getline(ss, speedStr, ',');
                std::getline(ss, sensitivityStr, ',');
                
                float speed = std::stof(speedStr);
                float sensitivity = std::stof(sensitivityStr);
                gameObject.AddComponent<MovementComponent>(speed, sensitivity);
            }
            else if (line.find("CameraComponent: ") == 0) {
                std::stringstream ss(line.substr(17));
                std::string fovStr, nearStr, farStr;
                std::getline(ss, fovStr, ',');
                std::getline(ss, nearStr, ',');
                std::getline(ss, farStr, ',');
                
                auto* camera = gameObject.AddComponent<CameraComponent>();
                camera->fieldOfView = std::stof(fovStr);
                camera->nearPlane = std::stof(nearStr);
                camera->farPlane = std::stof(farStr);
            }
            else if (line.find("LightComponent: ") == 0) {
                std::stringstream ss(line.substr(16));
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
            else if (line.find("MeshComponent: ") == 0) {
                std::stringstream ss(line.substr(15));
                std::string typeStr, rStr, gStr, bStr, metallicStr, roughnessStr, visibleStr;
                std::getline(ss, typeStr, ',');
                std::getline(ss, rStr, ',');
                std::getline(ss, gStr, ',');
                std::getline(ss, bStr, ',');
                std::getline(ss, metallicStr, ',');
                std::getline(ss, roughnessStr, ',');
                std::getline(ss, visibleStr, ',');
                
                auto* mesh = gameObject.AddComponent<MeshComponent>(typeStr);
                mesh->SetColor(Vector3(std::stof(rStr), std::stof(gStr), std::stof(bStr)));
                mesh->SetMetallic(std::stof(metallicStr));
                mesh->SetRoughness(std::stof(roughnessStr));
                mesh->SetVisible(std::stoi(visibleStr) != 0);
            }
            else if (line.find("ParentID: ") == 0) {
                Logger::Debug("Found ParentID entry during deserialization - will restore hierarchy in second pass");
            }
        }
        
        return gameObject;
    }
}
