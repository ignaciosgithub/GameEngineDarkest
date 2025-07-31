#pragma once

#include "../../Core/Math/Vector3.h"
#include "../../Physics/PhysicsWorld.h"
#include "../../Physics/RigidBody/RigidBody.h"
#include <vector>

namespace GameEngine {

class Light;
class World;

struct OcclusionRayHit {
    bool hit = false;
    Vector3 hitPoint;
    Vector3 hitNormal;
    float distance = 0.0f;
    RigidBody* hitBody = nullptr;
};

class LightOcclusion {
public:
    LightOcclusion();
    ~LightOcclusion();
    
    // Initialize with physics world for raycasting
    void Initialize(PhysicsWorld* physicsWorld);
    void Shutdown();
    
    // Main occlusion calculation
    float CalculateOcclusion(const Light* light, const Vector3& targetPoint, World* world);
    
    // Raycast against physics bodies for occlusion
    bool RaycastForOcclusion(const Vector3& start, const Vector3& end, OcclusionRayHit& hit);
    
    // Check if a light is occluded by walls
    bool IsLightOccluded(const Light* light, const Vector3& targetPoint, World* world);
    
    // Calculate shadow attenuation based on occlusion
    float CalculateShadowAttenuation(const Light* light, const Vector3& targetPoint, World* world);
    
    // Settings
    void SetOcclusionEnabled(bool enabled) { m_occlusionEnabled = enabled; }
    bool IsOcclusionEnabled() const { return m_occlusionEnabled; }
    
    void SetShadowSoftness(float softness) { m_shadowSoftness = softness; }
    float GetShadowSoftness() const { return m_shadowSoftness; }
    
    void SetMaxOcclusionDistance(float distance) { m_maxOcclusionDistance = distance; }
    float GetMaxOcclusionDistance() const { return m_maxOcclusionDistance; }
    
private:
    PhysicsWorld* m_physicsWorld = nullptr;
    bool m_occlusionEnabled = true;
    float m_shadowSoftness = 0.1f;
    float m_maxOcclusionDistance = 100.0f;
    
    // Helper methods
    std::vector<RigidBody*> GetOccludingBodies(World* world);
    bool IsBodyOccluding(RigidBody* body);
    float CalculateDistanceAttenuation(float distance, float maxDistance);
    
    // Multi-sample occlusion for soft shadows
    float CalculateSoftShadowOcclusion(const Light* light, const Vector3& targetPoint, World* world);
    std::vector<Vector3> GenerateSamplePoints(const Vector3& lightPos, const Vector3& targetPoint, int sampleCount);
};

}
