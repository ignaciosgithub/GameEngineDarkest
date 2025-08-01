#include "DebugRenderer.h"
#include "../../Core/Logging/Logger.h"
#include "../Core/OpenGLHeaders.h"
#include <vector>
#include <cmath>

namespace GameEngine {

bool DebugRenderer::s_initialized = false;

void DebugRenderer::Initialize() {
    if (s_initialized) {
        Logger::Warning("DebugRenderer already initialized");
        return;
    }
    
    SetupWireframeShader();
    s_initialized = true;
    Logger::Info("DebugRenderer initialized successfully");
}

void DebugRenderer::Shutdown() {
    if (!s_initialized) {
        Logger::Warning("DebugRenderer not initialized");
        return;
    }
    
    s_initialized = false;
    Logger::Info("DebugRenderer shutdown");
}

void DebugRenderer::RenderLightGizmo(const Light& light, const Vector3& position) {
    if (!s_initialized) {
        Logger::Warning("DebugRenderer not initialized");
        return;
    }
    
    Vector3 lightColor = light.GetColor();
    
    switch (light.GetType()) {
        case LightType::Point:
            RenderWireSphere(position, light.GetRange(), lightColor);
            break;
            
        case LightType::Spot:
            RenderWireCone(position, light.GetDirection(), light.GetOuterConeAngle(), light.GetRange(), lightColor);
            break;
            
        case LightType::Directional:
            RenderDirectionalArrow(position, light.GetDirection(), lightColor);
            break;
    }
}

void DebugRenderer::RenderWireSphere(const Vector3& center, float radius, const Vector3& color) {
    if (!s_initialized) return;
    
    std::vector<Vector3> vertices;
    std::vector<unsigned int> indices;
    
    const int segments = 16;
    const int rings = 8;
    
    for (int ring = 0; ring <= rings; ++ring) {
        float phi = static_cast<float>(ring) * M_PI / rings;
        float y = radius * cos(phi);
        float ringRadius = radius * sin(phi);
        
        for (int segment = 0; segment <= segments; ++segment) {
            float theta = static_cast<float>(segment) * 2.0f * M_PI / segments;
            float x = ringRadius * cos(theta);
            float z = ringRadius * sin(theta);
            
            vertices.push_back(center + Vector3(x, y, z));
        }
    }
    
    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            int current = ring * (segments + 1) + segment;
            int next = current + segments + 1;
            
            indices.push_back(current);
            indices.push_back(current + 1);
            
            if (ring < rings - 1) {
                indices.push_back(current);
                indices.push_back(next);
            }
        }
    }
    
    RenderWireframeMesh(vertices, indices, color);
}

void DebugRenderer::RenderWireCone(const Vector3& position, const Vector3& direction, float angle, float range, const Vector3& color) {
    if (!s_initialized) return;
    
    std::vector<Vector3> vertices;
    std::vector<unsigned int> indices;
    
    const int segments = 16;
    float angleRad = angle * M_PI / 180.0f;
    float coneRadius = range * tan(angleRad);
    
    vertices.push_back(position);
    
    Vector3 forward = direction.Normalized();
    Vector3 right = Vector3(0, 1, 0).Cross(forward).Normalized();
    Vector3 up = forward.Cross(right).Normalized();
    
    Vector3 baseCenter = position + forward * range;
    
    for (int i = 0; i <= segments; ++i) {
        float theta = static_cast<float>(i) * 2.0f * M_PI / segments;
        float x = coneRadius * cos(theta);
        float y = coneRadius * sin(theta);
        
        Vector3 basePoint = baseCenter + right * x + up * y;
        vertices.push_back(basePoint);
    }
    
    for (int i = 1; i <= segments; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        
        indices.push_back(i);
        indices.push_back(i == segments ? 1 : i + 1);
    }
    
    RenderWireframeMesh(vertices, indices, color);
}

void DebugRenderer::RenderDirectionalArrow(const Vector3& position, const Vector3& direction, const Vector3& color) {
    if (!s_initialized) return;
    
    std::vector<Vector3> vertices;
    std::vector<unsigned int> indices;
    
    Vector3 forward = direction.Normalized();
    Vector3 right = Vector3(0, 1, 0).Cross(forward).Normalized();
    Vector3 up = forward.Cross(right).Normalized();
    
    float arrowLength = 2.0f;
    float arrowHeadLength = 0.5f;
    float arrowHeadWidth = 0.2f;
    
    Vector3 start = position;
    Vector3 end = position + forward * arrowLength;
    
    vertices.push_back(start);
    vertices.push_back(end);
    
    Vector3 headBase = end - forward * arrowHeadLength;
    Vector3 headLeft = headBase + right * arrowHeadWidth;
    Vector3 headRight = headBase - right * arrowHeadWidth;
    Vector3 headUp = headBase + up * arrowHeadWidth;
    Vector3 headDown = headBase - up * arrowHeadWidth;
    
    vertices.push_back(headLeft);
    vertices.push_back(headRight);
    vertices.push_back(headUp);
    vertices.push_back(headDown);
    
    indices.push_back(0); indices.push_back(1); // Shaft
    indices.push_back(1); indices.push_back(2); // Head left
    indices.push_back(1); indices.push_back(3); // Head right
    indices.push_back(1); indices.push_back(4); // Head up
    indices.push_back(1); indices.push_back(5); // Head down
    
    RenderWireframeMesh(vertices, indices, color);
}

void DebugRenderer::SetupWireframeShader() {
    Logger::Debug("Wireframe shader setup - placeholder implementation");
}

void DebugRenderer::RenderWireframeMesh(const std::vector<Vector3>& vertices, const std::vector<unsigned int>& indices, const Vector3& color) {
    
    Logger::Debug("Rendering wireframe mesh with " + std::to_string(vertices.size()) + 
                 " vertices and " + std::to_string(indices.size()) + " indices");
}

}
