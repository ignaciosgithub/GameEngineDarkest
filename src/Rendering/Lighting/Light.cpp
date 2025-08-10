#include "Light.h"
#include "../../Core/Logging/Logger.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GameEngine {

Light::Light(LightType type) : m_type(type) {
    Logger::Info("Light created with type: " + std::to_string(static_cast<int>(type)));
    
    switch (type) {
        case LightType::Directional:
            m_data.direction = Vector3(0.0f, -1.0f, 0.0f);
            m_data.range = 1000.0f; // Large range for directional lights
            break;
        case LightType::Point:
            m_data.range = 10.0f;
            break;
        case LightType::Spot:
            m_data.direction = Vector3(0.0f, -1.0f, 0.0f);
            m_data.range = 10.0f;
            m_data.innerConeAngle = 30.0f;
            m_data.outerConeAngle = 45.0f;
            break;
    }
}

Light::~Light() {
    CleanupShadowMap();
    Logger::Debug("Light destroyed");
}

bool Light::InitializeShadowMap() {
    if (m_shadowMapInitialized) {
        Logger::Warning("Shadow map already initialized");
        return true;
    }

    if (!m_data.castShadows) {
        Logger::Info("Light does not cast shadows, skipping shadow map initialization");
        return true;
    }

    Logger::Info("Initializing shadow map with size: " + std::to_string(m_data.shadowMapSize));

    m_shadowFramebuffer = std::make_shared<FrameBuffer>(m_data.shadowMapSize, m_data.shadowMapSize);

    if (m_type == LightType::Point) {
        m_shadowMap = std::make_shared<Texture>();
        m_shadowMap->CreateEmptyCubeDepth(m_data.shadowMapSize, TextureFormat::Depth24);
    } else {
        m_shadowMap = std::make_shared<Texture>();
        m_shadowMap->CreateEmpty(m_data.shadowMapSize, m_data.shadowMapSize, TextureFormat::Depth24);
        m_shadowFramebuffer->AddDepthAttachment(TextureFormat::Depth24);
    }

    m_shadowMapInitialized = true;
    Logger::Info("Shadow map initialized successfully");
    return true;
}

void Light::CleanupShadowMap() {
    if (m_shadowMapInitialized) {
        Logger::Debug("Cleaning up shadow map");
        m_shadowFramebuffer.reset();
        m_shadowMap.reset();
        m_shadowMapInitialized = false;
    }
}

Matrix4 Light::GetLightSpaceMatrix() const {
    return GetProjectionMatrix() * GetViewMatrix();
}

Matrix4 Light::GetProjectionMatrix() const {
    switch (m_type) {
        case LightType::Directional: {
            float orthoSize = 25.0f;
            float nearPlane = m_data.shadowNearPlane;
            float farPlane = m_data.shadowFarPlane;
            return Matrix4::Orthographic(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
        }
        case LightType::Point: {
            float aspect = 1.0f;
            float fov = 90.0f * static_cast<float>(M_PI) / 180.0f;
            return Matrix4::Perspective(fov, aspect, m_data.shadowNearPlane, m_data.shadowFarPlane);
        }
        case LightType::Spot: {
            float aspect = 1.0f;
            float fov = std::max(1.0f, m_data.outerConeAngle * 2.0f) * static_cast<float>(M_PI) / 180.0f;
            return Matrix4::Perspective(fov, aspect, m_data.shadowNearPlane, m_data.shadowFarPlane);
        }
    }
    return Matrix4::Identity();
}

Matrix4 Light::GetViewMatrix() const {
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
    if (std::abs(m_data.direction.Dot(up)) > 0.99f) {
        up = Vector3(1.0f, 0.0f, 0.0f);
    }

    switch (m_type) {
        case LightType::Directional: {
            Vector3 lightPos = m_data.position - m_data.direction * 50.0f;
            Vector3 target = m_data.position;
            return Matrix4::LookAt(lightPos, target, up);
        }
        case LightType::Point:
        case LightType::Spot: {
            Vector3 target = m_data.position + m_data.direction;
            return Matrix4::LookAt(m_data.position, target, up);
        }
    }

    return Matrix4::Identity();
}

float Light::GetAttenuationAtDistance(float distance) const {
    if (m_type == LightType::Directional) {
        return 1.0f; // No attenuation for directional lights
    }
    
    if (distance >= m_data.range) {
        return 0.0f;
    }
    
    if (m_type == LightType::Point || m_type == LightType::Spot) {
        float minDistance = 0.01f;
        float effectiveDistance = std::max(distance, minDistance);
        
        float inverseSquareAttenuation = 1.0f / (effectiveDistance * effectiveDistance);
        
        float rangeFactor = 1.0f - (distance / m_data.range);
        rangeFactor = std::max(0.0f, rangeFactor);
        
        return inverseSquareAttenuation * rangeFactor;
    }
    
    return 1.0f;
}

bool Light::IsInRange(const Vector3& point) const {
    if (m_type == LightType::Directional) {
        return true; // Directional lights affect everything
    }
    
    float distance = (point - m_data.position).Length();
    return distance <= m_data.range;
}

float Light::GetInfluenceRadius() const {
    if (m_type == LightType::Directional) {
        return std::numeric_limits<float>::max();
    }
    
    return m_data.range;
}

float Light::GetSpotAttenuation(const Vector3& lightToPoint) const {
    if (m_type != LightType::Spot) {
        return 1.0f;
    }
    
    Vector3 lightDirection;
    if (m_data.direction.LengthSquared() > 0.0001f) {
        lightDirection = m_data.direction.Normalized();
    } else {
        lightDirection = Vector3(0.0f, -1.0f, 0.0f); // Default downward direction
    }
    
    Vector3 pointDirection;
    if (lightToPoint.LengthSquared() > 0.0001f) {
        pointDirection = lightToPoint.Normalized();
    } else {
        pointDirection = Vector3(0.0f, 1.0f, 0.0f); // Default upward direction
    }
    
    float cosAngle = lightDirection.Dot(pointDirection);
    float innerCos = std::cos(m_data.innerConeAngle * 0.5f * static_cast<float>(M_PI) / 180.0f);
    float outerCos = std::cos(m_data.outerConeAngle * 0.5f * static_cast<float>(M_PI) / 180.0f);
    
    if (cosAngle > innerCos) {
        return 1.0f; // Full intensity
    } else if (cosAngle > outerCos) {
        float factor = (cosAngle - outerCos) / (innerCos - outerCos);
        return factor * factor; // Quadratic falloff
    } else {
        return 0.0f; // Outside cone
    }
}

}
