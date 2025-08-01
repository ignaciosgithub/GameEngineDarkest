#include "LightManager.h"
#include "../../Core/Logging/Logger.h"
#include "../../Core/Components/TransformComponent.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {

LightManager::LightManager() {
    m_activeLights.reserve(MAX_LIGHTS);
    Logger::Debug("LightManager created");
}

void LightManager::Initialize(PhysicsWorld* physicsWorld) {
    m_lightOcclusion.Initialize(physicsWorld);
    Logger::Info("LightManager initialized with PhysicsWorld for occlusion");
}

LightManager::~LightManager() {
    Clear();
    Logger::Debug("LightManager destroyed");
}

void LightManager::CollectLights(World* world) {
    if (!world) {
        Logger::Warning("LightManager::CollectLights - World is null");
        return;
    }
    
    Clear();
    
    auto entities = world->GetEntities();
    std::vector<Entity> lightEntities;
    for (auto entity : entities) {
        if (world->HasComponent<LightComponent>(entity)) {
            lightEntities.push_back(entity);
        }
    }
    
    for (auto entity : lightEntities) {
        auto lightComponent = world->GetComponent<LightComponent>(entity);
        auto transformComponent = world->GetComponent<TransformComponent>(entity);
        
        if (lightComponent && transformComponent) {
            Light* light = &lightComponent->light;
            
            light->SetPosition(transformComponent->transform.GetPosition());
            
            if (m_activeLights.size() < MAX_LIGHTS) {
                m_activeLights.push_back(light);
            } else {
                Logger::Warning("Maximum number of lights (" + std::to_string(MAX_LIGHTS) + ") reached. Skipping additional lights.");
                break;
            }
        }
    }
    
    Logger::Debug("Collected " + std::to_string(m_activeLights.size()) + " lights");
}

float LightManager::CalculateTotalBrightness() const {
    float totalBrightness = 0.0f;
    
    for (const Light* light : m_activeLights) {
        if (light) {
            totalBrightness += light->GetIntensity();
        }
    }
    
    return totalBrightness;
}

void LightManager::ApplyBrightnessLimits() {
    float totalBrightness = CalculateTotalBrightness();
    
    if (totalBrightness > MAX_BRIGHTNESS) {
        float scaleFactor = MAX_BRIGHTNESS / totalBrightness;
        
        Logger::Debug("Total brightness (" + std::to_string(totalBrightness) + 
                     ") exceeds maximum (" + std::to_string(MAX_BRIGHTNESS) + 
                     "). Applying scale factor: " + std::to_string(scaleFactor));
        
        for (Light* light : m_activeLights) {
            if (light) {
                float currentIntensity = light->GetIntensity();
                light->SetIntensity(currentIntensity * scaleFactor);
            }
        }
    }
}

void LightManager::CullLights(const Vector3& cameraPosition, const Vector3& cameraDirection) {
    auto it = std::remove_if(m_activeLights.begin(), m_activeLights.end(),
        [this, &cameraPosition, &cameraDirection](const Light* light) {
            return !IsLightVisible(light, cameraPosition, cameraDirection);
        });
    
    m_activeLights.erase(it, m_activeLights.end());
    
    Logger::Debug("After culling: " + std::to_string(m_activeLights.size()) + " lights remain");
}

void LightManager::SortLightsByDistance(const Vector3& cameraPosition) {
    std::sort(m_activeLights.begin(), m_activeLights.end(),
        [&cameraPosition](const Light* a, const Light* b) {
            if (!a || !b) return false;
            
            float distA = (a->GetPosition() - cameraPosition).LengthSquared();
            float distB = (b->GetPosition() - cameraPosition).LengthSquared();
            
            return distA < distB;
        });
    
    Logger::Debug("Lights sorted by distance from camera");
}

void LightManager::GetShaderLightData(std::vector<ShaderLightData>& lightData) const {
    lightData.clear();
    lightData.reserve(m_activeLights.size());
    
    for (const Light* light : m_activeLights) {
        if (!light) continue;
        
        ShaderLightData data;
        data.position = light->GetPosition();
        data.direction = light->GetDirection();
        data.color = light->GetColor();
        data.intensity = light->GetIntensity();
        data.range = light->GetRange();
        data.innerConeAngle = light->GetInnerConeAngle();
        data.outerConeAngle = light->GetOuterConeAngle();
        data.castShadows = light->GetCastShadows();
        
        switch (light->GetType()) {
            case LightType::Directional:
                data.type = 0;
                break;
            case LightType::Point:
                data.type = 1;
                break;
            case LightType::Spot:
                data.type = 2;
                break;
            default:
                data.type = 0;
                break;
        }
        
        lightData.push_back(data);
    }
    
    Logger::Debug("Generated shader data for " + std::to_string(lightData.size()) + " lights");
}

float LightManager::CalculateLightContribution(const Light* light, const Vector3& targetPoint, World* world) {
    if (!light) return 0.0f;
    
    float baseContribution = 1.0f;
    
    if (light->GetType() == LightType::Point || light->GetType() == LightType::Spot) {
        float distance = (targetPoint - light->GetPosition()).Length();
        
        baseContribution = light->GetAttenuationAtDistance(distance);
        
        if (baseContribution <= 0.0f) {
            return 0.0f; // Outside light range or no contribution
        }
    }
    
    float occlusionFactor = m_lightOcclusion.CalculateShadowAttenuation(light, targetPoint, world);
    
    return baseContribution * occlusionFactor * light->GetIntensity();
}

void LightManager::CalculateDistanceFromCamera(const Vector3& cameraPosition) {
    for (const Light* light : m_activeLights) {
        if (light) {
            (void)(light->GetPosition() - cameraPosition).Length(); // Calculate but don't store for now
        }
    }
}

bool LightManager::IsLightVisible(const Light* light, const Vector3& cameraPosition, const Vector3& /* cameraDirection */) const {
    if (!light) return false;
    
    if (light->GetType() == LightType::Directional) {
        return true;
    }
    
    float distance = (light->GetPosition() - cameraPosition).Length();
    
    if (distance > light->GetRange() * 2.0f) { // Allow some extra range for smooth transitions
        return false;
    }
    
    
    return true;
}

}
