#pragma once

#include "../../Core/Math/Vector3.h"
#include "../../Core/Math/Matrix4.h"
#include "../../Core/ECS/Component.h"
#include "../Core/Texture.h"
#include "../Core/FrameBuffer.h"
#include <memory>

namespace GameEngine {

enum class LightType {
    Directional,
    Point,
    Spot
};

struct LightData {
    Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 direction = Vector3(0.0f, -1.0f, 0.0f);
    Vector3 color = Vector3(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
    float range = 10.0f;
    float innerConeAngle = 30.0f;  // For spot lights
    float outerConeAngle = 45.0f;  // For spot lights
    
    // Shadow properties
    bool castShadows = true;
    float shadowBias = 0.005f;
    float shadowNormalBias = 0.4f;
    int shadowMapSize = 1024;
    float shadowNearPlane = 0.1f;
    float shadowFarPlane = 100.0f;
};

class Light : public Component<Light> {
public:
    Light(LightType type = LightType::Directional);
    ~Light() override;
    
    // Light type
    void SetType(LightType type) { m_type = type; }
    LightType GetType() const { return m_type; }
    
    // Light properties
    void SetPosition(const Vector3& position) { m_data.position = position; }
    void SetDirection(const Vector3& direction) { m_data.direction = direction.Normalized(); }
    void SetColor(const Vector3& color) { m_data.color = color; }
    void SetIntensity(float intensity) { m_data.intensity = intensity; }
    void SetRange(float range) { m_data.range = range; }
    void SetSpotAngles(float innerAngle, float outerAngle) { 
        m_data.innerConeAngle = innerAngle; 
        m_data.outerConeAngle = outerAngle; 
    }
    
    // Getters
    const Vector3& GetPosition() const { return m_data.position; }
    const Vector3& GetDirection() const { return m_data.direction; }
    const Vector3& GetColor() const { return m_data.color; }
    float GetIntensity() const { return m_data.intensity; }
    float GetRange() const { return m_data.range; }
    float GetInnerConeAngle() const { return m_data.innerConeAngle; }
    float GetOuterConeAngle() const { return m_data.outerConeAngle; }
    
    const LightData& GetData() const { return m_data; }
    LightData& GetData() { return m_data; }
    
    // Shadow mapping
    void SetCastShadows(bool cast) { m_data.castShadows = cast; }
    bool GetCastShadows() const { return m_data.castShadows; }
    
    void SetShadowBias(float bias) { m_data.shadowBias = bias; }
    float GetShadowBias() const { return m_data.shadowBias; }
    
    void SetShadowMapSize(int size) { m_data.shadowMapSize = size; }
    int GetShadowMapSize() const { return m_data.shadowMapSize; }
    
    // Shadow map management
    bool InitializeShadowMap();
    void CleanupShadowMap();
    std::shared_ptr<Texture> GetShadowMap() const { return m_shadowMap; }
    std::shared_ptr<FrameBuffer> GetShadowFramebuffer() const { return m_shadowFramebuffer; }
    
    // Shadow matrix calculation
    Matrix4 GetLightSpaceMatrix() const;
    Matrix4 GetProjectionMatrix() const;
    Matrix4 GetViewMatrix() const;
    
    // Utility functions
    float GetAttenuationAtDistance(float distance) const;
    Vector3 GetFinalColor() const { return m_data.color * m_data.intensity; }
    
    // Light culling helpers
    bool IsInRange(const Vector3& point) const;
    float GetInfluenceRadius() const;
    float GetSpotAttenuation(const Vector3& lightToPoint) const;
    
private:
    LightType m_type;
    LightData m_data;
    
    // Shadow mapping resources
    std::shared_ptr<FrameBuffer> m_shadowFramebuffer;
    std::shared_ptr<Texture> m_shadowMap;
    bool m_shadowMapInitialized = false;
};

// Light component for ECS
class LightComponent : public Component<LightComponent> {
public:
    LightComponent(LightType type = LightType::Directional) : light(type) {}
    ~LightComponent() override = default;
    
    Light light;
};

}
