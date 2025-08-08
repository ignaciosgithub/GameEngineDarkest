#include "TestSceneManager.h"
#include "../Components/TransformComponent.h"
#include "../Components/CameraComponent.h"
#include "../Logging/Logger.h"
#include "../../Rendering/Meshes/Mesh.h"
#include <cmath>

namespace GameEngine {

TestSceneManager::TestSceneManager(World* world, RenderManager* renderManager)
    : m_world(world), m_renderManager(renderManager), m_currentSceneType(TestSceneType::BasicLighting) {
    Logger::Info("TestSceneManager initialized");
}

TestSceneManager::~TestSceneManager() {
    ClearCurrentScene();
    Logger::Info("TestSceneManager destroyed");
}

void TestSceneManager::LoadScene(TestSceneType sceneType) {
    Logger::Info("Loading test scene: " + std::to_string(static_cast<int>(sceneType)));
    
    ReplaceScene();
    m_currentSceneType = sceneType;
    m_animationTime = 0.0f;
    
    switch (sceneType) {
        case TestSceneType::BasicLighting:
            CreateBasicLightingScene();
            m_currentSceneName = "Basic Lighting Test";
            break;
        case TestSceneType::MultipleLight:
            CreateMultipleLightScene();
            m_currentSceneName = "Multiple Lights Test";
            break;
        case TestSceneType::PBRMaterials:
            CreatePBRMaterialsScene();
            m_currentSceneName = "PBR Materials Test";
            break;
        case TestSceneType::PostProcessing:
            CreatePostProcessingScene();
            m_currentSceneName = "Post-Processing Test";
            break;
        case TestSceneType::Raytracing:
            CreateRaytracingScene();
            m_currentSceneName = "Raytracing Test";
            break;
    }
    
    Logger::Info("Test scene loaded: " + m_currentSceneName);
}

void TestSceneManager::Update(float deltaTime) {
    if (m_enableAnimation) {
        m_animationTime += deltaTime;
        
        switch (m_currentSceneType) {
            case TestSceneType::MultipleLight: {
                for (size_t i = 0; i < m_sceneEntities.size(); ++i) {
                    Entity entity = m_sceneEntities[i];
                    if (m_world->HasComponent<LightComponent>(entity)) {
                        auto* lightComp = m_world->GetComponent<LightComponent>(entity);
                        if (lightComp && lightComp->light.GetType() == LightType::Point) {
                            float angle = m_animationTime + i * 2.0f;
                            Vector3 newPos = Vector3(
                                std::cos(angle) * 5.0f,
                                2.0f + std::sin(angle * 2.0f) * 1.0f,
                                std::sin(angle) * 5.0f
                            );
                            lightComp->light.SetPosition(newPos);
                        }
                    }
                }
                break;
            }
            case TestSceneType::PostProcessing: {
                for (Entity entity : m_sceneEntities) {
                    if (m_world->HasComponent<LightComponent>(entity)) {
                        auto* lightComp = m_world->GetComponent<LightComponent>(entity);
                        if (lightComp) {
                            float intensity = 1.0f + std::sin(m_animationTime * 2.0f) * 0.5f;
                            lightComp->light.SetIntensity(intensity);
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    }
}

void TestSceneManager::SwitchRenderingPipeline(RenderPipelineType pipelineType) {
    if (m_renderManager) {
        m_renderManager->SetPipeline(pipelineType);
        Logger::Info("Switched to rendering pipeline: " + std::to_string(static_cast<int>(pipelineType)));
    }
}

void TestSceneManager::CreateBasicLightingScene() {
    Logger::Info("Creating basic lighting test scene");
    
    AddTestPlane(Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f));
    
    AddTestCube(Vector3(-2.0f, 0.0f, 0.0f));
    AddTestCube(Vector3(0.0f, 0.0f, 0.0f));
    AddTestCube(Vector3(2.0f, 0.0f, 0.0f));
    
    CreateDirectionalLight(Vector3(-0.3f, -1.0f, -0.3f), Vector3(1.0f, 0.95f, 0.8f), 1.0f);
    
    CreatePointLight(Vector3(0.0f, 3.0f, 0.0f), Vector3(1.0f, 0.8f, 0.6f), 2.0f, 8.0f);
}

void TestSceneManager::CreateMultipleLightScene() {
    Logger::Info("Creating multiple lights test scene");
    
    AddTestPlane(Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f));
    
    for (int x = -2; x <= 2; ++x) {
        for (int z = -2; z <= 2; ++z) {
            AddTestCube(Vector3(x * 2.0f, 0.0f, z * 2.0f));
        }
    }
    
    CreatePointLight(Vector3(4.0f, 2.0f, 0.0f), Vector3(1.0f, 0.2f, 0.2f), 3.0f, 10.0f);  // Red
    CreatePointLight(Vector3(-4.0f, 2.0f, 0.0f), Vector3(0.2f, 1.0f, 0.2f), 3.0f, 10.0f); // Green
    CreatePointLight(Vector3(0.0f, 2.0f, 4.0f), Vector3(0.2f, 0.2f, 1.0f), 3.0f, 10.0f);  // Blue
    CreatePointLight(Vector3(0.0f, 2.0f, -4.0f), Vector3(1.0f, 1.0f, 0.2f), 3.0f, 10.0f); // Yellow
    
    CreateDirectionalLight(Vector3(-0.2f, -1.0f, -0.2f), Vector3(0.3f, 0.3f, 0.4f), 0.5f);
}

void TestSceneManager::CreatePBRMaterialsScene() {
    Logger::Info("Creating PBR materials test scene");
    
    AddTestPlane(Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f));
    
    auto material1 = CreatePBRMaterial(Vector3(0.8f, 0.2f, 0.2f), 0.0f, 0.1f);  // Smooth plastic
    auto material2 = CreatePBRMaterial(Vector3(0.8f, 0.2f, 0.2f), 0.0f, 0.5f);  // Rough plastic
    auto material3 = CreatePBRMaterial(Vector3(0.8f, 0.2f, 0.2f), 0.0f, 0.9f);  // Very rough plastic
    auto material4 = CreatePBRMaterial(Vector3(0.8f, 0.8f, 0.8f), 1.0f, 0.1f);  // Smooth metal
    auto material5 = CreatePBRMaterial(Vector3(0.8f, 0.8f, 0.8f), 1.0f, 0.5f);  // Rough metal
    
    AddTestSphere(Vector3(-4.0f, 0.0f, 0.0f), 1.0f, material1.get());
    AddTestSphere(Vector3(-2.0f, 0.0f, 0.0f), 1.0f, material2.get());
    AddTestSphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f, material3.get());
    AddTestSphere(Vector3(2.0f, 0.0f, 0.0f), 1.0f, material4.get());
    AddTestSphere(Vector3(4.0f, 0.0f, 0.0f), 1.0f, material5.get());
    
    CreateDirectionalLight(Vector3(-0.3f, -1.0f, -0.3f), Vector3(1.0f, 1.0f, 1.0f), 2.0f);
    
    CreatePointLight(Vector3(0.0f, 4.0f, 2.0f), Vector3(1.0f, 1.0f, 1.0f), 5.0f, 15.0f);
}

void TestSceneManager::CreatePostProcessingScene() {
    Logger::Info("Creating post-processing test scene");
    
    AddTestPlane(Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f));
    
    auto emissiveMaterial1 = CreateEmissiveMaterial(Vector3(1.0f, 0.2f, 0.2f), 3.0f);
    auto emissiveMaterial2 = CreateEmissiveMaterial(Vector3(0.2f, 1.0f, 0.2f), 3.0f);
    auto emissiveMaterial3 = CreateEmissiveMaterial(Vector3(0.2f, 0.2f, 1.0f), 3.0f);
    
    AddTestCube(Vector3(-2.0f, 1.0f, 0.0f), Vector3(1.0f), emissiveMaterial1.get());
    AddTestCube(Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f), emissiveMaterial2.get());
    AddTestCube(Vector3(2.0f, 1.0f, 0.0f), Vector3(1.0f), emissiveMaterial3.get());
    
    AddTestCube(Vector3(-1.0f, 0.0f, 2.0f));
    AddTestCube(Vector3(1.0f, 0.0f, 2.0f));
    
    CreatePointLight(Vector3(-2.0f, 3.0f, 0.0f), Vector3(1.0f, 0.2f, 0.2f), 5.0f, 8.0f);
    CreatePointLight(Vector3(2.0f, 3.0f, 0.0f), Vector3(0.2f, 0.2f, 1.0f), 5.0f, 8.0f);
    
    CreateDirectionalLight(Vector3(-0.2f, -1.0f, -0.2f), Vector3(0.2f, 0.2f, 0.3f), 0.3f);
}

void TestSceneManager::CreateRaytracingScene() {
    Logger::Info("Creating raytracing test scene");
    
    auto reflectiveMaterial = CreateReflectiveMaterial(Vector3(0.8f, 0.8f, 0.9f), 0.6f);
    AddTestPlane(Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f));
    
    auto mirrorMaterial = CreateReflectiveMaterial(Vector3(0.9f, 0.9f, 0.9f), 0.9f);
    AddTestSphere(Vector3(-2.0f, 0.0f, 0.0f), 1.0f, mirrorMaterial.get());
    AddTestSphere(Vector3(2.0f, 0.0f, 0.0f), 1.0f, mirrorMaterial.get());
    
    auto redReflective = CreateReflectiveMaterial(Vector3(0.8f, 0.2f, 0.2f), 0.7f);
    auto blueReflective = CreateReflectiveMaterial(Vector3(0.2f, 0.2f, 0.8f), 0.7f);
    AddTestCube(Vector3(0.0f, 0.0f, -2.0f), Vector3(1.0f), redReflective.get());
    AddTestCube(Vector3(0.0f, 0.0f, 2.0f), Vector3(1.0f), blueReflective.get());
    
    CreateDirectionalLight(Vector3(-0.5f, -1.0f, -0.3f), Vector3(1.0f, 1.0f, 1.0f), 2.0f);
    
    CreatePointLight(Vector3(0.0f, 5.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f), 8.0f, 20.0f);
}

void TestSceneManager::ReplaceScene() {
    std::vector<Entity> toDestroy;
    bool preservedCamera = false;
    Entity preservedCameraEntity;
    for (const auto& entity : m_world->GetEntities()) {
        if (!preservedCamera && m_world->HasComponent<CameraComponent>(entity)) {
            preservedCamera = true;
            preservedCameraEntity = entity;
            continue;
        }
        toDestroy.push_back(entity);
    }
    for (Entity e : toDestroy) {
        if (e.IsValid()) {
            m_world->DestroyEntity(e);
        }
    }
    m_sceneEntities.clear();
    m_sceneMaterials.clear();
    if (preservedCamera) {
        m_sceneEntities.push_back(preservedCameraEntity);
    }
}
void TestSceneManager::ReplaceScene() {
    std::vector<Entity> toDestroy;
    bool preservedCamera = false;
    Entity preservedCameraEntity;
    for (const auto& entity : m_world->GetEntities()) {
        if (!preservedCamera && m_world->HasComponent<CameraComponent>(entity)) {
            preservedCamera = true;
            preservedCameraEntity = entity;
            continue;
        }
        toDestroy.push_back(entity);
    }
    for (Entity e : toDestroy) {
        if (e.IsValid()) {
            m_world->DestroyEntity(e);
        }
    }
    m_sceneEntities.clear();
    m_sceneMaterials.clear();
    if (preservedCamera) {
        m_sceneEntities.push_back(preservedCameraEntity);
    }
}
void TestSceneManager::ClearCurrentScene() {
    Logger::Debug("Clearing current test scene");
    
    for (Entity entity : m_sceneEntities) {
        if (entity.IsValid()) {
            m_world->DestroyEntity(entity);
        }
    }
    m_sceneEntities.clear();
    
    m_sceneMaterials.clear();
}

void TestSceneManager::AddTestCube(const Vector3& position, const Vector3& scale, Material* /*material*/) {
    Entity entity = m_world->CreateEntity();
    m_world->AddComponent<TransformComponent>(entity, position, Quaternion(), scale);
    
    m_sceneEntities.push_back(entity);
    
    Logger::Debug("Added test cube at position: " + std::to_string(position.x) + ", " + 
                  std::to_string(position.y) + ", " + std::to_string(position.z));
}

void TestSceneManager::AddTestSphere(const Vector3& position, float radius, Material* /*material*/) {
    Entity entity = m_world->CreateEntity();
    m_world->AddComponent<TransformComponent>(entity, position, Quaternion(), Vector3(radius));
    
    m_sceneEntities.push_back(entity);
    
    Logger::Debug("Added test sphere at position: " + std::to_string(position.x) + ", " + 
                  std::to_string(position.y) + ", " + std::to_string(position.z));
}

void TestSceneManager::AddTestPlane(const Vector3& position, const Vector3& scale) {
    Entity entity = m_world->CreateEntity();
    m_world->AddComponent<TransformComponent>(entity, position, Quaternion(), scale);
    
    m_sceneEntities.push_back(entity);
    
    Logger::Debug("Added test plane at position: " + std::to_string(position.x) + ", " + 
                  std::to_string(position.y) + ", " + std::to_string(position.z));
}

Entity TestSceneManager::CreateDirectionalLight(const Vector3& direction, const Vector3& color, float intensity) {
    Entity entity = m_world->CreateEntity();
    m_world->AddComponent<TransformComponent>(entity, Vector3(0.0f));
    
    auto* lightComp = m_world->AddComponent<LightComponent>(entity, LightType::Directional);
    lightComp->light.SetDirection(direction);
    lightComp->light.SetColor(color);
    lightComp->light.SetIntensity(intensity);
    
    m_sceneEntities.push_back(entity);
    
    Logger::Debug("Created directional light with intensity: " + std::to_string(intensity));
    return entity;
}

Entity TestSceneManager::CreatePointLight(const Vector3& position, const Vector3& color, float intensity, float range) {
    Entity entity = m_world->CreateEntity();
    m_world->AddComponent<TransformComponent>(entity, position);
    
    auto* lightComp = m_world->AddComponent<LightComponent>(entity, LightType::Point);
    lightComp->light.SetPosition(position);
    lightComp->light.SetColor(color);
    lightComp->light.SetIntensity(intensity);
    lightComp->light.SetRange(range);
    
    m_sceneEntities.push_back(entity);
    
    Logger::Debug("Created point light at position: " + std::to_string(position.x) + ", " + 
                  std::to_string(position.y) + ", " + std::to_string(position.z));
    return entity;
}

Entity TestSceneManager::CreateSpotLight(const Vector3& position, const Vector3& direction, const Vector3& color, float intensity) {
    Entity entity = m_world->CreateEntity();
    m_world->AddComponent<TransformComponent>(entity, position);
    
    auto* lightComp = m_world->AddComponent<LightComponent>(entity, LightType::Spot);
    lightComp->light.SetPosition(position);
    lightComp->light.SetDirection(direction);
    lightComp->light.SetColor(color);
    lightComp->light.SetIntensity(intensity);
    lightComp->light.SetSpotAngles(30.0f, 45.0f);
    
    m_sceneEntities.push_back(entity);
    
    Logger::Debug("Created spot light at position: " + std::to_string(position.x) + ", " + 
                  std::to_string(position.y) + ", " + std::to_string(position.z));
    return entity;
}

std::shared_ptr<Material> TestSceneManager::CreatePBRMaterial(const Vector3& albedo, float metallic, float roughness) {
    auto material = std::make_shared<Material>();
    material->SetAlbedo(albedo);
    material->SetMetallic(metallic);
    material->SetRoughness(roughness);
    
    m_sceneMaterials.push_back(material);
    
    Logger::Debug("Created PBR material - Metallic: " + std::to_string(metallic) + 
                  ", Roughness: " + std::to_string(roughness));
    return material;
}

std::shared_ptr<Material> TestSceneManager::CreateReflectiveMaterial(const Vector3& albedo, float reflectivity) {
    auto material = std::make_shared<Material>();
    material->SetAlbedo(albedo);
    material->SetMetallic(1.0f);
    material->SetRoughness(1.0f - reflectivity);
    
    m_sceneMaterials.push_back(material);
    
    Logger::Debug("Created reflective material with reflectivity: " + std::to_string(reflectivity));
    return material;
}

std::shared_ptr<Material> TestSceneManager::CreateEmissiveMaterial(const Vector3& color, float intensity) {
    auto material = std::make_shared<Material>();
    material->SetAlbedo(color);
    material->SetEmission(color * intensity);
    
    m_sceneMaterials.push_back(material);
    
    Logger::Debug("Created emissive material with intensity: " + std::to_string(intensity));
    return material;
}

}
