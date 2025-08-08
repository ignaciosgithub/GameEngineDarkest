#pragma once

#include "../ECS/World.h"
#include "../../Rendering/RenderManager.h"
#include "../../Rendering/Lighting/Light.h"
#include "../../Rendering/Materials/Material.h"
#include <memory>
#include <string>
#include <vector>

namespace GameEngine {

enum class TestSceneType {
    BasicLighting,
    MultipleLight,
    PBRMaterials,
    PostProcessing,
    Raytracing
};

class TestSceneManager {
public:
    TestSceneManager(World* world, RenderManager* renderManager);
    ~TestSceneManager();
    
    void LoadScene(TestSceneType sceneType);
    void Update(float deltaTime);
    void SwitchRenderingPipeline(RenderPipelineType pipelineType);
    
    // Scene creation methods
    void CreateBasicLightingScene();
    void CreateMultipleLightScene();
    void CreatePBRMaterialsScene();
    void CreatePostProcessingScene();
    void CreateRaytracingScene();
    
    void ReplaceScene();
    // Utility methods
    void ClearCurrentScene();
    void AddTestCube(const Vector3& position, const Vector3& scale = Vector3(1.0f), Material* material = nullptr);
    void AddTestSphere(const Vector3& position, float radius = 1.0f, Material* material = nullptr);
    void AddTestPlane(const Vector3& position, const Vector3& scale = Vector3(10.0f, 1.0f, 10.0f));
    
    // Lighting setup
    Entity CreateDirectionalLight(const Vector3& direction, const Vector3& color, float intensity = 1.0f);
    Entity CreatePointLight(const Vector3& position, const Vector3& color, float intensity = 1.0f, float range = 10.0f);
    Entity CreateSpotLight(const Vector3& position, const Vector3& direction, const Vector3& color, float intensity = 1.0f);
    
    // Material creation
    std::shared_ptr<Material> CreatePBRMaterial(const Vector3& albedo, float metallic, float roughness);
    std::shared_ptr<Material> CreateReflectiveMaterial(const Vector3& albedo, float reflectivity = 0.8f);
    std::shared_ptr<Material> CreateEmissiveMaterial(const Vector3& color, float intensity = 2.0f);
    
    TestSceneType GetCurrentSceneType() const { return m_currentSceneType; }
    const std::string& GetCurrentSceneName() const { return m_currentSceneName; }
    
private:
    World* m_world;
    RenderManager* m_renderManager;
    TestSceneType m_currentSceneType;
    std::string m_currentSceneName;
    
    // Scene entities for cleanup
    std::vector<Entity> m_sceneEntities;
    std::vector<std::shared_ptr<Material>> m_sceneMaterials;
    
    // Animation state
    float m_animationTime = 0.0f;
    bool m_enableAnimation = true;
};

}
