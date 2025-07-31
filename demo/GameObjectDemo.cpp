#include "../src/Core/Engine.h"
#include "../src/Core/GameObject/GameObject.h"
#include "../src/Core/GameObject/Prefab.h"
#include "../src/Core/Scenes/Scene.h"
#include "../src/Core/Components/MeshComponent.h"
#include "../src/Core/Components/TransformComponent.h"
#include "../src/Core/Components/MovementComponent.h"
#include "../src/Core/Components/CameraComponent.h"
#include "../src/Core/Logging/Logger.h"

using namespace GameEngine;

class GameObjectDemo {
public:
    GameObjectDemo() : m_engine(nullptr), m_scene(nullptr) {}
    
    bool Initialize() {
        Logger::Info("=== GameObject Demo Starting ===");
        
        m_engine = std::make_unique<Engine>();
        if (!m_engine->Initialize()) {
            Logger::Error("Failed to initialize engine");
            return false;
        }
        
        m_scene = std::make_unique<Scene>(m_engine->GetWorld(), "GameObject Demo Scene");
        Logger::Info("Created demo scene: " + m_scene->GetName());
        
        CreateDemoGameObjects();
        
        CreateDemoPrefabs();
        
        TestOBJLoading();
        
        Logger::Info("GameObject Demo initialized successfully");
        Logger::Info("Scene contains " + std::to_string(m_scene->GetGameObjectCount()) + " GameObjects");
        
        return true;
    }
    
    void Run() {
        Logger::Info("=== Running GameObject Demo ===");
        
        for (int frame = 0; frame < 10; ++frame) {
            Logger::Info("--- Frame " + std::to_string(frame + 1) + " ---");
            
            LogGameObjectStates();
            
            Logger::Info("Frame " + std::to_string(frame + 1) + " completed");
        }
        
        Logger::Info("GameObject Demo completed successfully");
    }
    
    void Shutdown() {
        Logger::Info("=== Shutting down GameObject Demo ===");
        
        if (m_scene) {
            Logger::Info("Clearing scene with " + std::to_string(m_scene->GetGameObjectCount()) + " GameObjects");
            m_scene->Clear();
        }
        
        if (m_engine) {
            m_engine->Shutdown();
        }
        
        Logger::Info("GameObject Demo shutdown complete");
    }
    
private:
    std::unique_ptr<Engine> m_engine;
    std::unique_ptr<Scene> m_scene;
    std::vector<GameObject> m_demoObjects;
    std::vector<std::shared_ptr<Prefab>> m_demoPrefabs;
    
    void CreateDemoGameObjects() {
        Logger::Info("Creating demo GameObjects...");
        
        GameObject cubeObject = m_scene->CreateGameObject(Vector3(0.0f, 0.0f, 0.0f), "Demo Cube");
        auto* meshComp = cubeObject.AddComponent<MeshComponent>("cube");
        meshComp->SetColor(Vector3(1.0f, 0.0f, 0.0f)); // Red cube
        m_demoObjects.push_back(cubeObject);
        Logger::Info("Created cube GameObject at origin");
        
        GameObject sphereObject = m_scene->CreateGameObject(Vector3(3.0f, 0.0f, 0.0f), "Demo Sphere");
        auto* sphereMesh = sphereObject.AddComponent<MeshComponent>("sphere");
        sphereMesh->SetColor(Vector3(0.0f, 1.0f, 0.0f)); // Green sphere
        m_demoObjects.push_back(sphereObject);
        Logger::Info("Created sphere GameObject at (3, 0, 0)");
        
        GameObject planeObject = m_scene->CreateGameObject(Vector3(0.0f, -2.0f, 0.0f), "Demo Plane");
        auto* planeMesh = planeObject.AddComponent<MeshComponent>("plane");
        planeMesh->SetColor(Vector3(0.8f, 0.8f, 0.8f)); // Gray plane
        m_demoObjects.push_back(planeObject);
        Logger::Info("Created plane GameObject at (0, -2, 0)");
        
        GameObject movingObject = m_scene->CreateGameObject(Vector3(-3.0f, 0.0f, 0.0f), "Moving Cube");
        auto* movingMesh = movingObject.AddComponent<MeshComponent>("cube");
        movingMesh->SetColor(Vector3(0.0f, 0.0f, 1.0f)); // Blue cube
        auto* movement = movingObject.AddComponent<MovementComponent>();
        movement->velocity = Vector3(0.1f, 0.0f, 0.0f); // Move right slowly
        m_demoObjects.push_back(movingObject);
        Logger::Info("Created moving cube GameObject with MovementComponent");
        
        GameObject cameraObject = m_scene->CreateGameObject(Vector3(0.0f, 5.0f, 10.0f), "Demo Camera");
        auto* camera = cameraObject.AddComponent<CameraComponent>();
        camera->fieldOfView = 45.0f;
        camera->nearPlane = 0.1f;
        camera->farPlane = 100.0f;
        m_demoObjects.push_back(cameraObject);
        Logger::Info("Created camera GameObject looking at origin");
    }
    
    void CreateDemoPrefabs() {
        Logger::Info("Creating demo prefabs...");
        
        auto cubePrefab = std::make_shared<Prefab>();
        cubePrefab->SetName("Colored Cube Prefab");
        cubePrefab->AddComponentData("MeshComponent", "cube");
        cubePrefab->SetTransformData(Vector3::Zero, Quaternion::Identity(), Vector3::One);
        m_demoPrefabs.push_back(cubePrefab);
        Logger::Info("Created Colored Cube prefab");
        
        for (int i = 0; i < 3; ++i) {
            Vector3 position(i * 2.0f, 2.0f, -3.0f);
            GameObject instance = m_scene->InstantiatePrefab(cubePrefab, position);
            
            auto* meshComp = instance.GetComponent<MeshComponent>();
            if (meshComp) {
                float hue = i / 3.0f;
                meshComp->SetColor(Vector3(hue, 1.0f - hue, 0.5f));
            }
            
            m_demoObjects.push_back(instance);
            Logger::Info("Instantiated prefab at position (" + std::to_string(position.x) + ", " + 
                        std::to_string(position.y) + ", " + std::to_string(position.z) + ")");
        }
        
        auto spherePrefab = std::make_shared<Prefab>();
        spherePrefab->SetName("Textured Sphere Prefab");
        spherePrefab->AddComponentData("MeshComponent", "sphere");
        spherePrefab->SetTransformData(Vector3(0.0f, 1.0f, 0.0f), Quaternion::Identity(), Vector3::One);
        m_demoPrefabs.push_back(spherePrefab);
        Logger::Info("Created Textured Sphere prefab");
        
        GameObject sphereInstance = m_scene->InstantiatePrefab(spherePrefab, Vector3(0.0f, 4.0f, -5.0f));
        auto* sphereMeshComp = sphereInstance.GetComponent<MeshComponent>();
        if (sphereMeshComp) {
            sphereMeshComp->SetColor(Vector3(1.0f, 1.0f, 0.0f)); // Yellow sphere
            sphereMeshComp->SetMetallic(0.8f);
            sphereMeshComp->SetRoughness(0.2f);
        }
        m_demoObjects.push_back(sphereInstance);
        Logger::Info("Instantiated sphere prefab with metallic material");
    }
    
    void TestOBJLoading() {
        Logger::Info("Testing OBJ loading capabilities...");
        
        GameObject objObject = m_scene->CreateGameObject(Vector3(5.0f, 0.0f, 0.0f), "OBJ Test Object");
        auto* objMesh = objObject.AddComponent<MeshComponent>();
        
        objMesh->LoadMeshFromOBJ("assets/models/test.obj");
        objMesh->SetColor(Vector3(0.5f, 0.0f, 0.5f)); // Purple color
        
        m_demoObjects.push_back(objObject);
        Logger::Info("Created OBJ test object (will use cube if OBJ file not found)");
    }
    
    void LogGameObjectStates() {
        for (size_t i = 0; i < std::min(m_demoObjects.size(), size_t(3)); ++i) {
            const GameObject& obj = m_demoObjects[i];
            if (obj.IsValid()) {
                auto* transform = obj.GetTransform();
                auto* mesh = obj.GetComponent<MeshComponent>();
                
                if (transform && mesh) {
                    Vector3 pos = transform->transform.GetPosition();
                    Logger::Debug("GameObject " + std::to_string(i) + " at (" + 
                                std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + 
                                std::to_string(pos.z) + ") with mesh type: " + mesh->GetMeshType());
                }
            }
        }
    }
};

int main() {
    Logger::Info("=== GameEngine GameObject Demo ===");
    
    GameObjectDemo demo;
    
    if (!demo.Initialize()) {
        Logger::Error("Failed to initialize GameObject demo");
        return -1;
    }
    
    demo.Run();
    demo.Shutdown();
    
    Logger::Info("GameObject demo completed successfully");
    return 0;
}
