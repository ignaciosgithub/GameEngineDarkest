#include "Light.h"
#include "../../Core/Logging/Logger.h"
#include <cmath>
#include <algorithm>

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
    
    m_shadowMap = std::make_shared<Texture>();
    m_shadowMap->CreateEmpty(m_data.shadowMapSize, m_data.shadowMapSize, TextureFormat::Depth24);
    
    m_shadowFramebuffer = std::make_shared<FrameBuffer>(m_data.shadowMapSize, m_data.shadowMapSize);
    m_shadowFramebuffer->AddDepthAttachment(TextureFormat::Depth24);
    
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
    Matrix4 projection;
    
    switch (m_type) {
        case LightType::Directional: {
            // float orthoSize = 50.0f; // This should be calculated based on scene bounds
            projection = Matrix4(); // Placeholder orthographic projection
            break;
        }
        case LightType::Point: {
            projection = Matrix4(); // Placeholder perspective projection
            break;
        }
        case LightType::Spot: {
            // float fov = m_data.outerConeAngle * 2.0f;
            projection = Matrix4(); // Placeholder perspective projection
            break;
        }
    }
    
    return projection;
}

Matrix4 Light::GetViewMatrix() const {
    Matrix4 view;
    
    switch (m_type) {
        case LightType::Directional: {
            // Vector3 lightPos = m_data.position - m_data.direction * 100.0f;
            // Vector3 target = m_data.position;
            Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
            
            if (std::abs(m_data.direction.Dot(up)) > 0.99f) {
                up = Vector3(1.0f, 0.0f, 0.0f);
            }
            
            view = Matrix4(); // Placeholder look-at matrix
            break;
        }
        case LightType::Point:
        case LightType::Spot: {
            // Vector3 target = m_data.position + m_data.direction;
            Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
            
            if (std::abs(m_data.direction.Dot(up)) > 0.99f) {
                up = Vector3(1.0f, 0.0f, 0.0f);
            }
            
            view = Matrix4(); // Placeholder look-at matrix
            break;
        }
    }
    
    return view;
}

float Light::GetAttenuationAtDistance(float distance) const {
    if (m_type == LightType::Directional) {
        return 1.0f; // No attenuation for directional lights
    }
    
    if (distance >= m_data.range) {
        return 0.0f;
    }
    
    float constant = 1.0f;
    float linear = 2.0f / m_data.range;
    float quadratic = 1.0f / (m_data.range * m_data.range);
    
    float attenuation = 1.0f / (constant + linear * distance + quadratic * distance * distance);
    
    if (m_type == LightType::Spot) {
    }
    
    return attenuation;
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
    
    Vector3 lightDirection = m_data.direction.Normalized();
    Vector3 pointDirection = lightToPoint.Normalized();
    
    float cosAngle = lightDirection.Dot(pointDirection);
    float innerCos = std::cos(m_data.innerConeAngle * 0.5f * M_PI / 180.0f);
    float outerCos = std::cos(m_data.outerConeAngle * 0.5f * M_PI / 180.0f);
    
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
