#pragma once

#include "Light.h"
#include "LightOcclusion.h"
#include "../../Core/ECS/World.h"
#include <vector>
#include <memory>

namespace GameEngine {

class LightManager {
public:
    LightManager();
    ~LightManager();
    
    // Initialize with physics world for occlusion
    void Initialize(PhysicsWorld* physicsWorld);
    
    // Light collection and management
    void CollectLights(World* world);
    const std::vector<Light*>& GetActiveLights() const { return m_activeLights; }
    
    // Brightness calculation and hardware limits
    float CalculateTotalBrightness() const;
    void ApplyBrightnessLimits();
    
    // Light culling and optimization
    void CullLights(const Vector3& cameraPosition, const Vector3& cameraDirection);
    void SortLightsByDistance(const Vector3& cameraPosition);
    
    // Utility methods
    int GetActiveLightCount() const { return static_cast<int>(m_activeLights.size()); }
    bool HasLights() const { return !m_activeLights.empty(); }
    void Clear() { m_activeLights.clear(); }
    
    // Light data for shaders
    struct ShaderLightData {
        Vector3 position;
        Vector3 direction;
        Vector3 color;
        float intensity;
        float range;
        int type; // 0=directional, 1=point, 2=spot
        float innerConeAngle;
        float outerConeAngle;
        bool castShadows;
    };
    
    void GetShaderLightData(std::vector<ShaderLightData>& lightData) const;
    
    // Occlusion system
    LightOcclusion* GetLightOcclusion() { return &m_lightOcclusion; }
    const LightOcclusion* GetLightOcclusion() const { return &m_lightOcclusion; }
    
    // Calculate light contribution with occlusion
    float CalculateLightContribution(const Light* light, const Vector3& targetPoint, World* world);
    
private:
    std::vector<Light*> m_activeLights;
    float m_totalBrightness = 0.0f;
    LightOcclusion m_lightOcclusion;
    
    // Helper methods
    void CalculateDistanceFromCamera(const Vector3& cameraPosition);
    bool IsLightVisible(const Light* light, const Vector3& cameraPosition, const Vector3& cameraDirection) const;
};

}
