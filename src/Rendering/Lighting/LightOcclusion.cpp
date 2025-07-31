#include "LightOcclusion.h"
#include "Light.h"
#include "../../Core/ECS/World.h"
#include "../../Core/Components/RigidBodyComponent.h"
#include "../../Core/Logging/Logger.h"
#include "../../Physics/Collision/ContinuousCollisionDetection.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {

LightOcclusion::LightOcclusion() {
    Logger::Debug("LightOcclusion created");
}

LightOcclusion::~LightOcclusion() {
    Shutdown();
    Logger::Debug("LightOcclusion destroyed");
}

void LightOcclusion::Initialize(PhysicsWorld* physicsWorld) {
    if (!physicsWorld) {
        Logger::Error("LightOcclusion::Initialize - PhysicsWorld is null");
        return;
    }
    
    m_physicsWorld = physicsWorld;
    Logger::Info("LightOcclusion initialized with PhysicsWorld");
}

void LightOcclusion::Shutdown() {
    m_physicsWorld = nullptr;
    Logger::Debug("LightOcclusion shutdown complete");
}

float LightOcclusion::CalculateOcclusion(const Light* light, const Vector3& targetPoint, World* world) {
    if (!m_occlusionEnabled || !light || !world || !m_physicsWorld) {
        return 1.0f; // No occlusion
    }
    
    Vector3 lightPos = light->GetPosition();
    if (light->GetType() == LightType::Directional) {
        Vector3 lightDir = light->GetDirection().Normalized();
        lightPos = targetPoint - lightDir * m_maxOcclusionDistance;
    }
    
    float distance = (targetPoint - lightPos).Length();
    if (distance > m_maxOcclusionDistance) {
        return 0.0f; // Too far, fully occluded
    }
    
    if (m_shadowSoftness > 0.0f) {
        return CalculateSoftShadowOcclusion(light, targetPoint, world);
    }
    
    OcclusionRayHit hit;
    if (RaycastForOcclusion(lightPos, targetPoint, hit)) {
        if (hit.distance < distance - 0.01f) { // Small epsilon to avoid self-intersection
            return 0.0f; // Fully occluded
        }
    }
    
    return 1.0f; // Not occluded
}

bool LightOcclusion::RaycastForOcclusion(const Vector3& start, const Vector3& end, OcclusionRayHit& hit) {
    if (!m_physicsWorld) {
        return false;
    }
    
    hit.hit = false;
    hit.distance = (end - start).Length();
    
    auto occludingBodies = GetOccludingBodies(nullptr); // We'll get all bodies for now
    
    float closestDistance = hit.distance;
    bool foundHit = false;
    
    for (RigidBody* body : occludingBodies) {
        if (!body || !IsBodyOccluding(body)) continue;
        
        ContinuousCollisionInfo collisionInfo;
        if (ContinuousCollisionDetection::RaycastAgainstBody(start, end, body, collisionInfo)) {
            float hitDistance = collisionInfo.timeOfImpact * (end - start).Length();
            
            if (hitDistance < closestDistance) {
                closestDistance = hitDistance;
                hit.hit = true;
                hit.hitPoint = collisionInfo.contactPoint;
                hit.hitNormal = collisionInfo.normal;
                hit.distance = hitDistance;
                hit.hitBody = body;
                foundHit = true;
            }
        }
    }
    
    return foundHit;
}

bool LightOcclusion::IsLightOccluded(const Light* light, const Vector3& targetPoint, World* world) {
    float occlusion = CalculateOcclusion(light, targetPoint, world);
    return occlusion < 0.5f; // Consider occluded if less than 50% light reaches target
}

float LightOcclusion::CalculateShadowAttenuation(const Light* light, const Vector3& targetPoint, World* world) {
    if (!m_occlusionEnabled) {
        return 1.0f; // No shadow attenuation
    }
    
    float occlusion = CalculateOcclusion(light, targetPoint, world);
    
    if (light->GetType() == LightType::Point || light->GetType() == LightType::Spot) {
        float distance = (targetPoint - light->GetPosition()).Length();
        float distanceAttenuation = CalculateDistanceAttenuation(distance, light->GetRange());
        occlusion *= distanceAttenuation;
    }
    
    return occlusion;
}

std::vector<RigidBody*> LightOcclusion::GetOccludingBodies(World* /* world */) {
    std::vector<RigidBody*> occludingBodies;
    
    
    
    return occludingBodies;
}

bool LightOcclusion::IsBodyOccluding(RigidBody* body) {
    if (!body) return false;
    
    return body->IsStatic();
}

float LightOcclusion::CalculateDistanceAttenuation(float distance, float maxDistance) {
    if (distance >= maxDistance) {
        return 0.0f;
    }
    
    float attenuation = 1.0f - (distance * distance) / (maxDistance * maxDistance);
    return std::max(0.0f, attenuation);
}

float LightOcclusion::CalculateSoftShadowOcclusion(const Light* light, const Vector3& targetPoint, World* /* world */) {
    const int sampleCount = 4; // Number of samples for soft shadows
    
    Vector3 lightPos = light->GetPosition();
    if (light->GetType() == LightType::Directional) {
        Vector3 lightDir = light->GetDirection().Normalized();
        lightPos = targetPoint - lightDir * m_maxOcclusionDistance;
    }
    
    std::vector<Vector3> samplePoints = GenerateSamplePoints(lightPos, targetPoint, sampleCount);
    
    float totalOcclusion = 0.0f;
    int validSamples = 0;
    
    for (const Vector3& samplePoint : samplePoints) {
        OcclusionRayHit hit;
        float sampleOcclusion = 1.0f;
        
        if (RaycastForOcclusion(samplePoint, targetPoint, hit)) {
            float distance = (targetPoint - samplePoint).Length();
            if (hit.distance < distance - 0.01f) {
                sampleOcclusion = 0.0f;
            }
        }
        
        totalOcclusion += sampleOcclusion;
        validSamples++;
    }
    
    if (validSamples == 0) {
        return 1.0f; // No valid samples, assume no occlusion
    }
    
    return totalOcclusion / validSamples;
}

std::vector<Vector3> LightOcclusion::GenerateSamplePoints(const Vector3& lightPos, const Vector3& targetPoint, int sampleCount) {
    std::vector<Vector3> samplePoints;
    samplePoints.reserve(sampleCount);
    
    samplePoints.push_back(lightPos);
    
    if (sampleCount <= 1) {
        return samplePoints;
    }
    
    Vector3 toLightDir = (lightPos - targetPoint).Normalized();
    Vector3 perpendicular1 = toLightDir.Cross(Vector3::Up).Normalized();
    Vector3 perpendicular2 = toLightDir.Cross(perpendicular1).Normalized();
    
    float sampleRadius = m_shadowSoftness;
    
    for (int i = 1; i < sampleCount; ++i) {
        float angle = (2.0f * 3.14159f * i) / (sampleCount - 1);
        float radius = sampleRadius * std::sqrt(static_cast<float>(i) / (sampleCount - 1));
        
        Vector3 offset = perpendicular1 * (radius * std::cos(angle)) + 
                        perpendicular2 * (radius * std::sin(angle));
        
        samplePoints.push_back(lightPos + offset);
    }
    
    return samplePoints;
}

}
