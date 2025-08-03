#include "DebugRenderer.h"
#include "../../Core/Logging/Logger.h"
#include "../Core/OpenGLHeaders.h"
#include <vector>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
        float phi = static_cast<float>(ring) * static_cast<float>(M_PI) / rings;
        float y = radius * cos(phi);
        float ringRadius = radius * sin(phi);
        
        for (int segment = 0; segment <= segments; ++segment) {
            float theta = static_cast<float>(segment) * 2.0f * static_cast<float>(M_PI) / segments;
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
    float angleRad = angle * static_cast<float>(M_PI) / 180.0f;
    float coneRadius = range * tan(angleRad);
    
    vertices.push_back(position);
    
    Vector3 forward = direction.Normalized();
    Vector3 right = Vector3(0, 1, 0).Cross(forward).Normalized();
    Vector3 up = forward.Cross(right).Normalized();
    
    Vector3 baseCenter = position + forward * range;
    
    for (int i = 0; i <= segments; ++i) {
        float theta = static_cast<float>(i) * 2.0f * static_cast<float>(M_PI) / segments;
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

void DebugRenderer::RenderSelectionOutline(const Vector3& center, const Vector3& size, const Vector3& color) {
    if (!s_initialized) return;
    
    std::vector<Vector3> vertices;
    std::vector<unsigned int> indices;
    
    Vector3 halfSize = size * 0.5f;
    
    vertices.push_back(center + Vector3(-halfSize.x, -halfSize.y, -halfSize.z)); // 0: left-bottom-back
    vertices.push_back(center + Vector3( halfSize.x, -halfSize.y, -halfSize.z)); // 1: right-bottom-back
    vertices.push_back(center + Vector3( halfSize.x,  halfSize.y, -halfSize.z)); // 2: right-top-back
    vertices.push_back(center + Vector3(-halfSize.x,  halfSize.y, -halfSize.z)); // 3: left-top-back
    vertices.push_back(center + Vector3(-halfSize.x, -halfSize.y,  halfSize.z)); // 4: left-bottom-front
    vertices.push_back(center + Vector3( halfSize.x, -halfSize.y,  halfSize.z)); // 5: right-bottom-front
    vertices.push_back(center + Vector3( halfSize.x,  halfSize.y,  halfSize.z)); // 6: right-top-front
    vertices.push_back(center + Vector3(-halfSize.x,  halfSize.y,  halfSize.z)); // 7: left-top-front
    
    indices.push_back(0); indices.push_back(1);
    indices.push_back(1); indices.push_back(2);
    indices.push_back(2); indices.push_back(3);
    indices.push_back(3); indices.push_back(0);
    
    indices.push_back(4); indices.push_back(5);
    indices.push_back(5); indices.push_back(6);
    indices.push_back(6); indices.push_back(7);
    indices.push_back(7); indices.push_back(4);
    
    indices.push_back(0); indices.push_back(4);
    indices.push_back(1); indices.push_back(5);
    indices.push_back(2); indices.push_back(6);
    indices.push_back(3); indices.push_back(7);
    
    RenderWireframeMesh(vertices, indices, color);
}

void DebugRenderer::RenderMovementGizmo(const Vector3& position, const Vector3& objectSize) {
    if (!s_initialized) return;
    
    float maxDimension = std::max({objectSize.x, objectSize.y, objectSize.z});
    float gizmoSize = maxDimension * 2.0f;
    
    if (gizmoSize < 1.0f) {
        gizmoSize = 1.0f;
    }
    
    Vector3 xEnd = position + Vector3(gizmoSize, 0, 0);
    std::vector<Vector3> xVertices = {position, xEnd};
    std::vector<unsigned int> xIndices = {0, 1};
    RenderWireframeMesh(xVertices, xIndices, Vector3(1.0f, 0.0f, 0.0f));
    
    Vector3 yEnd = position + Vector3(0, gizmoSize, 0);
    std::vector<Vector3> yVertices = {position, yEnd};
    std::vector<unsigned int> yIndices = {0, 1};
    RenderWireframeMesh(yVertices, yIndices, Vector3(0.0f, 1.0f, 0.0f));
    
    Vector3 zEnd = position + Vector3(0, 0, gizmoSize);
    std::vector<Vector3> zVertices = {position, zEnd};
    std::vector<unsigned int> zIndices = {0, 1};
    RenderWireframeMesh(zVertices, zIndices, Vector3(0.0f, 0.0f, 1.0f));
    
    float arrowHeadSize = gizmoSize * 0.15f;
    
    std::vector<Vector3> xHeadVertices = {
        xEnd,
        xEnd + Vector3(-arrowHeadSize, arrowHeadSize * 0.5f, 0),
        xEnd + Vector3(-arrowHeadSize, -arrowHeadSize * 0.5f, 0),
        xEnd + Vector3(-arrowHeadSize, 0, arrowHeadSize * 0.5f),
        xEnd + Vector3(-arrowHeadSize, 0, -arrowHeadSize * 0.5f)
    };
    std::vector<unsigned int> xHeadIndices = {0, 1, 0, 2, 0, 3, 0, 4};
    RenderWireframeMesh(xHeadVertices, xHeadIndices, Vector3(1.0f, 0.0f, 0.0f));
    
    std::vector<Vector3> yHeadVertices = {
        yEnd,
        yEnd + Vector3(arrowHeadSize * 0.5f, -arrowHeadSize, 0),
        yEnd + Vector3(-arrowHeadSize * 0.5f, -arrowHeadSize, 0),
        yEnd + Vector3(0, -arrowHeadSize, arrowHeadSize * 0.5f),
        yEnd + Vector3(0, -arrowHeadSize, -arrowHeadSize * 0.5f)
    };
    std::vector<unsigned int> yHeadIndices = {0, 1, 0, 2, 0, 3, 0, 4};
    RenderWireframeMesh(yHeadVertices, yHeadIndices, Vector3(0.0f, 1.0f, 0.0f));
    
    std::vector<Vector3> zHeadVertices = {
        zEnd,
        zEnd + Vector3(arrowHeadSize * 0.5f, 0, -arrowHeadSize),
        zEnd + Vector3(-arrowHeadSize * 0.5f, 0, -arrowHeadSize),
        zEnd + Vector3(0, arrowHeadSize * 0.5f, -arrowHeadSize),
        zEnd + Vector3(0, -arrowHeadSize * 0.5f, -arrowHeadSize)
    };
    std::vector<unsigned int> zHeadIndices = {0, 1, 0, 2, 0, 3, 0, 4};
    RenderWireframeMesh(zHeadVertices, zHeadIndices, Vector3(0.0f, 0.0f, 1.0f));
}

void DebugRenderer::SetupWireframeShader() {
    Logger::Debug("Wireframe shader setup - unlit debug rendering implementation");
}

void DebugRenderer::RenderWireframeMesh(const std::vector<Vector3>& vertices, const std::vector<unsigned int>& indices, const Vector3& color) {
    if (vertices.empty() || indices.empty()) return;
    
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vector3), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), (void*)0);
    glEnableVertexAttribArray(0);
    
    glEnable(GL_DEPTH_TEST);
    glLineWidth(3.0f);
    
    glColor3f(color.x, color.y, color.z);
    glDrawElements(GL_LINES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    
    glLineWidth(1.0f);
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    
    Logger::Debug("Rendered wireframe mesh with " + std::to_string(vertices.size()) + 
                 " vertices and " + std::to_string(indices.size()) + " indices");
}

}
