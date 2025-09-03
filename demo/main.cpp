#include "Core/Engine.h"
#include "Core/Logging/Logger.h"
#include "Core/Scenes/Scene.h"
#include "Core/GameObject/GameObject.h"
#include "Core/Math/Vector3.h"
#include <iostream>
#include "Core/Components/MeshComponent.h"
#include "Core/Components/ColliderComponent.h"
#include "Core/Components/RigidBodyComponent.h"
#include "Rendering/Lighting/Light.h"
#include "Core/Components/CameraComponent.h"



int main() {
    try {
        GameEngine::Engine engine;
        
        if (!engine.Initialize("GameEngine Multi-Light Occlusion Demo", 1280, 720)) {
            GameEngine::Logger::Error("Failed to initialize engine");
            return -1;
        }
        
        GameEngine::Logger::Info("=== Starting Multi-Light Demo ===");
        GameEngine::Logger::Info("Features demonstrated:");
        GameEngine::Logger::Info("- Multiple point lights with different colors and intensities");
        GameEngine::Logger::Info("- Real-time light property changes and animations");
        GameEngine::Logger::Info("- Light occlusion through collision geometry");
        GameEngine::Logger::Info("- Light accumulation with brightness clamping (MAX_BRIGHTNESS = 100.0f)");
        GameEngine::Logger::Info("- Point light shadows and directional lighting");
        GameEngine::Logger::Info("- Forward and Deferred rendering pipeline support");
        GameEngine::Logger::Info("");
        GameEngine::Logger::Info("Controls:");
        GameEngine::Logger::Info("- WASD: Move camera");
        GameEngine::Logger::Info("- Mouse: Look around");
        GameEngine::Logger::Info("- 1: Switch to BasicLighting scene with Deferred rendering");
        GameEngine::Logger::Info("- 2: Switch to MultipleLight scene with Forward rendering");
        GameEngine::Logger::Info("- 3: Switch to PBRMaterials scene with Deferred rendering");
        GameEngine::Logger::Info("- 4: Switch to PostProcessing scene with Forward rendering");
        GameEngine::Logger::Info("- 5: Switch to Raytracing scene with Raytracing pipeline");
        GameEngine::Logger::Info("- ESC: Exit demo");
        GameEngine::Logger::Info("");
        GameEngine::Logger::Info("The engine automatically creates a 5x5 cube grid with multiple lights:");
        GameEngine::Logger::Info("- Dynamic point lights with different colors (red, green, blue)");
        GameEngine::Logger::Info("- Animated light movement and intensity changes");
        GameEngine::Logger::Info("- Directional sun light for ambient illumination");
        GameEngine::Logger::Info("- Light occlusion system preventing light bleeding through walls");
        GameEngine::Logger::Info("- Shadow mapping for realistic lighting effects");
        GameEngine::Logger::Info("");
        GameEngine::Logger::Info("=== GameObject Hierarchy Demo ===");
        GameEngine::Logger::Info("Testing parent-child relationships and transform inheritance:");
        
        auto* world = engine.GetWorld();
        if (world) {
            {
                GameEngine::Entity ground = world->CreateEntity();
                world->AddComponent<GameEngine::TransformComponent>(ground, GameEngine::Vector3(0.0f, -1.0f, 0.0f),
                                                                    GameEngine::Quaternion(), GameEngine::Vector3(40.0f, 1.0f, 40.0f));
                auto* mesh = world->AddComponent<GameEngine::MeshComponent>(ground, "plane");
                if (mesh) mesh->SetColor(GameEngine::Vector3(0.6f, 0.6f, 0.6f));
                auto* col = world->AddComponent<GameEngine::ColliderComponent>(ground);
                if (col) col->SetBoxCollider(GameEngine::Vector3(1.0f, 0.1f, 1.0f));
                auto* rb = world->AddComponent<GameEngine::RigidBodyComponent>(ground);
                if (rb && rb->GetRigidBody()) rb->GetRigidBody()->SetMass(0.0f);
            }
            {
                GameEngine::Entity cube = world->CreateEntity();
                world->AddComponent<GameEngine::TransformComponent>(cube, GameEngine::Vector3(0.0f, 0.5f, 0.0f),
                                                                    GameEngine::Quaternion(), GameEngine::Vector3(1.0f, 1.0f, 1.0f));
                auto* mesh = world->AddComponent<GameEngine::MeshComponent>(cube, "cube");
                if (mesh) mesh->SetColor(GameEngine::Vector3(0.85f, 0.85f, 0.9f));
                auto* col = world->AddComponent<GameEngine::ColliderComponent>(cube);
                if (col) col->SetBoxCollider(GameEngine::Vector3(1.0f, 1.0f, 1.0f));
            }
            {
                GameEngine::Entity light = world->CreateEntity();
                world->AddComponent<GameEngine::TransformComponent>(light, GameEngine::Vector3(1.5f, 3.0f, 2.0f));
                auto* lc = world->AddComponent<GameEngine::LightComponent>(light, GameEngine::LightType::Point);
                lc->light.SetPosition(GameEngine::Vector3(1.5f, 3.0f, 2.0f));
                lc->light.SetColor(GameEngine::Vector3(1.0f, 0.95f, 0.8f));
                lc->light.SetIntensity(2.8f);
                lc->light.SetRange(15.0f);
                lc->light.SetCastShadows(true);
            }
            {
                GameEngine::Entity cam = world->CreateEntity();
                world->AddComponent<GameEngine::TransformComponent>(cam,
                    GameEngine::Vector3(6.0f, 4.0f, 8.0f),
                    GameEngine::Quaternion(),
                    GameEngine::Vector3(1.0f, 1.0f, 1.0f));
                auto* camera = world->AddComponent<GameEngine::CameraComponent>(cam);
                if (camera) {
                    camera->fieldOfView = 45.0f;
                    camera->nearPlane = 0.1f;
                    camera->farPlane = 100.0f;
                }
            }
            {
                GameEngine::Entity sun = world->CreateEntity();
                world->AddComponent<GameEngine::TransformComponent>(sun, GameEngine::Vector3(0.0f, 0.0f, 0.0f));
                auto* dl = world->AddComponent<GameEngine::LightComponent>(sun, GameEngine::LightType::Directional);
                if (dl) {
                    dl->light.SetDirection(GameEngine::Vector3(-0.4f, -1.0f, -0.3f));
                    dl->light.SetColor(GameEngine::Vector3(1.0f, 1.0f, 1.0f));
                    dl->light.SetIntensity(0.2f);
                    dl->light.SetCastShadows(false);
                }
            }

        }

        
        

        if (world) {
            GameEngine::Scene scene(world, "HierarchyTestScene");
            
            GameEngine::GameObject parent = scene.CreateGameObject(GameEngine::Vector3(0, 0, 0), "Parent");
            GameEngine::GameObject child1 = scene.CreateGameObject(GameEngine::Vector3(2, 0, 0), "Child1");
            GameEngine::GameObject child2 = scene.CreateGameObject(GameEngine::Vector3(-2, 0, 0), "Child2");
            
            child1.SetParent(&parent);
            child2.SetParent(&parent);
            
            GameEngine::Logger::Info("Created GameObject hierarchy: Parent with 2 children");
            GameEngine::Logger::Info("- Parent at (0, 0, 0)");
            GameEngine::Logger::Info("- Child1 at (2, 0, 0) relative to parent");
            GameEngine::Logger::Info("- Child2 at (-2, 0, 0) relative to parent");
            
            if (auto* parentTransform = parent.GetTransform()) {
                parentTransform->transform.Rotate(GameEngine::Vector3(0, 1, 0), 45.0f * (3.14159f / 180.0f));
                GameEngine::Logger::Info("Rotated parent by 45 degrees around Y-axis");
                GameEngine::Logger::Info("Children should inherit this rotation automatically");
            }
            
            auto rootObjects = scene.GetRootGameObjects();
            GameEngine::Logger::Info("Found " + std::to_string(rootObjects.size()) + " root GameObjects");
            
            auto children = scene.FindChildrenOf(&parent);
            GameEngine::Logger::Info("Parent has " + std::to_string(children.size()) + " children");
            
            if (scene.SaveToFile("hierarchy_test.scene")) {
                GameEngine::Logger::Info("Saved hierarchy test scene to file");
            }
            
            GameEngine::Logger::Info("GameObject hierarchy demonstration completed successfully");
        } else {
            GameEngine::Logger::Error("Could not access World for hierarchy demonstration");
        }
        
        GameEngine::Logger::Info("");
        
        engine.Run();
        engine.Shutdown();
        
        return 0;
    }
    catch (const std::exception& e) {
        GameEngine::Logger::Error("Exception caught: " + std::string(e.what()));
        return -1;
    }
    catch (...) {
        GameEngine::Logger::Error("Unknown exception caught");
        return -1;
    }
}
