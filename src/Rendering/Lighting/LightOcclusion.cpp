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
        return 1.0f;
    }

    Vector3 lightPos;
    Vector3 lightDir;
    float maxDist = m_maxOcclusionDistance;

    switch (light->GetType()) {
        case LightType::Directional: {
            lightDir = light->GetDirection().Normalized();
            lightPos = targetPoint - lightDir * maxDist;
            break;
        }
        case LightType::Point:
        case LightType::Spot: {
            lightPos = light->GetPosition();
            lightDir = (targetPoint - lightPos).Normalized();
            maxDist = (targetPoint - lightPos).Length();
            if (maxDist > light->GetRange()) {
                return 0.0f;
            }
            break;
        }
        default:
            lightPos = targetPoint;
            lightDir = Vector3::Zero;
            break;
    }

    if (light->GetType() == LightType::Spot) {
        Vector3 spotDir = light->GetDirection().Normalized();
        float d = std::max(-1.0f, std::min(1.0f, lightDir.Dot(spotDir)));
        float angle = std::acos(d);
        float outer = light->GetOuterConeAngle() * (3.14159265f / 180.0f);
        if (angle > outer) {
            return 0.0f;
        }
    }

    if (m_shadowSoftness > 0.0f) {
        return CalculateSoftShadowOcclusion(light, targetPoint, world);
    }

    OcclusionRayHit hit;
    if (RaycastForOcclusion(lightPos, targetPoint, hit)) {
        if (hit.distance < maxDist - 0.01f) {
            return 0.0f;
        }
    }

    return 1.0f;
}

bool LightOcclusion::RaycastForOcclusion(const Vector3& start, const Vector3& end, OcclusionRayHit& hit) {
    if (!m_physicsWorld) {
        return false;
    }

    hit.hit = false;
    float totalLen = (end - start).Length();
    hit.distance = totalLen;

    auto occludingBodies = GetOccludingBodies(nullptr);

    float closestDistance = totalLen;
    bool foundHit = false;

    for (RigidBody* body : occludingBodies) {
        if (!body || !IsBodyOccluding(body)) continue;

        ContinuousCollisionInfo collisionInfo;
        if (ContinuousCollisionDetection::RaycastAgainstBody(start, end, body, collisionInfo)) {
            float hitDistance = std::max(0.0f, std::min(totalLen, collisionInfo.timeOfImpact * totalLen));
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
    if (!m_physicsWorld) return occludingBodies;
    const auto& bodies = m_physicsWorld->GetRigidBodies();
    occludingBodies.reserve(bodies.size());
    for (RigidBody* b : bodies) {
        if (b && IsBodyOccluding(b)) occludingBodies.push_back(b);
    }
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
    const int sampleCount = 6;
    Vector3 baseLightPos;
    Vector3 baseDir;
    float baseMaxDist = m_maxOcclusionDistance;

    if (light->GetType() == LightType::Directional) {
        baseDir = light->GetDirection().Normalized();
        baseLightPos = targetPoint - baseDir * baseMaxDist;
    } else {
        baseLightPos = light->GetPosition();
        baseDir = (targetPoint - baseLightPos).Normalized();
        baseMaxDist = (targetPoint - baseLightPos).Length();
        if (baseMaxDist > light->GetRange()) return 0.0f;
    }

    std::vector<Vector3> samplePoints = GenerateSamplePoints(baseLightPos, targetPoint, sampleCount);

    int litSamples = 0;
    for (const Vector3& sp : samplePoints) {
        OcclusionRayHit hit;
        bool blocked = false;
        if (RaycastForOcclusion(sp, targetPoint, hit)) {
            float d = (targetPoint - sp).Length();
            if (hit.distance < d - 0.01f) blocked = true;
        }
        if (!blocked) ++litSamples;
    }

    float visibility = (float)litSamples / (float)std::max(1, sampleCount);

    if (light->GetType() == LightType::Point || light->GetType() == LightType::Spot) {
        float dist = (targetPoint - light->GetPosition()).Length();
        visibility *= CalculateDistanceAttenuation(dist, light->GetRange());
    }

    if (light->GetType() == LightType::Spot) {
        Vector3 spotDir = light->GetDirection().Normalized();
        Vector3 L = (targetPoint - light->GetPosition()).Normalized();
        float d = std::max(-1.0f, std::min(1.0f, L.Dot(spotDir)));
        float angle = std::acos(d);
        float inner = light->GetInnerConeAngle() * (3.14159265f / 180.0f);
        float outer = light->GetOuterConeAngle() * (3.14159265f / 180.0f);
        if (angle > outer) {
            visibility = 0.0f;
        } else if (angle > inner) {
            float t = (angle - inner) / std::max(1e-4f, (outer - inner));
            visibility *= (1.0f - t);
        }
    }

    return visibility;
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
