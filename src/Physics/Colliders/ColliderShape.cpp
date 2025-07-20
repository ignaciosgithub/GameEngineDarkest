#include "ColliderShape.h"
#include "../../Core/Math/Vector3.h"
#include "../../Core/Math/Quaternion.h"
#include <algorithm>
#include <cmath>
#include <cfloat>

namespace GameEngine {

Vector3 SphereCollider::GetSupportPoint(const Vector3& direction) const {
    Vector3 normalizedDir = direction;
    normalizedDir.Normalize();
    return normalizedDir * m_radius;
}

void SphereCollider::GetAABB(const Vector3& position, const Quaternion& /*rotation*/, Vector3& min, Vector3& max) const {
    Vector3 radiusVec(m_radius, m_radius, m_radius);
    min = position - radiusVec;
    max = position + radiusVec;
}

float SphereCollider::GetVolume() const {
    return (4.0f / 3.0f) * 3.14159f * m_radius * m_radius * m_radius;
}

Vector3 SphereCollider::GetCenterOfMass() const {
    return Vector3::Zero;
}

Vector3 BoxCollider::GetSupportPoint(const Vector3& direction) const {
    Vector3 result;
    result.x = (direction.x >= 0.0f) ? m_halfExtents.x : -m_halfExtents.x;
    result.y = (direction.y >= 0.0f) ? m_halfExtents.y : -m_halfExtents.y;
    result.z = (direction.z >= 0.0f) ? m_halfExtents.z : -m_halfExtents.z;
    return result;
}

void BoxCollider::GetAABB(const Vector3& position, const Quaternion& rotation, Vector3& min, Vector3& max) const {
    Vector3 corners[8] = {
        Vector3(-m_halfExtents.x, -m_halfExtents.y, -m_halfExtents.z),
        Vector3( m_halfExtents.x, -m_halfExtents.y, -m_halfExtents.z),
        Vector3(-m_halfExtents.x,  m_halfExtents.y, -m_halfExtents.z),
        Vector3( m_halfExtents.x,  m_halfExtents.y, -m_halfExtents.z),
        Vector3(-m_halfExtents.x, -m_halfExtents.y,  m_halfExtents.z),
        Vector3( m_halfExtents.x, -m_halfExtents.y,  m_halfExtents.z),
        Vector3(-m_halfExtents.x,  m_halfExtents.y,  m_halfExtents.z),
        Vector3( m_halfExtents.x,  m_halfExtents.y,  m_halfExtents.z)
    };
    
    min = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    max = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    
    for (int i = 0; i < 8; ++i) {
        Vector3 worldCorner = position + rotation.RotateVector(corners[i]);
        min.x = std::min(min.x, worldCorner.x);
        min.y = std::min(min.y, worldCorner.y);
        min.z = std::min(min.z, worldCorner.z);
        max.x = std::max(max.x, worldCorner.x);
        max.y = std::max(max.y, worldCorner.y);
        max.z = std::max(max.z, worldCorner.z);
    }
}

float BoxCollider::GetVolume() const {
    return 8.0f * m_halfExtents.x * m_halfExtents.y * m_halfExtents.z;
}

Vector3 BoxCollider::GetCenterOfMass() const {
    return Vector3::Zero;
}

Vector3 CapsuleCollider::GetSupportPoint(const Vector3& direction) const {
    Vector3 normalizedDir = direction;
    normalizedDir.Normalize();
    
    float halfHeight = m_height * 0.5f;
    Vector3 topCenter(0, halfHeight, 0);
    Vector3 bottomCenter(0, -halfHeight, 0);
    
    Vector3 center = (direction.y >= 0.0f) ? topCenter : bottomCenter;
    
    return center + normalizedDir * m_radius;
}

void CapsuleCollider::GetAABB(const Vector3& position, const Quaternion& rotation, Vector3& min, Vector3& max) const {
    float halfHeight = m_height * 0.5f;
    Vector3 upVector = rotation.RotateVector(Vector3::Up);
    Vector3 topCenter = position + upVector * halfHeight;
    Vector3 bottomCenter = position - upVector * halfHeight;
    
    Vector3 radiusVec(m_radius, m_radius, m_radius);
    
    Vector3 topMin = topCenter - radiusVec;
    Vector3 topMax = topCenter + radiusVec;
    Vector3 bottomMin = bottomCenter - radiusVec;
    Vector3 bottomMax = bottomCenter + radiusVec;
    
    min.x = std::min(topMin.x, bottomMin.x);
    min.y = std::min(topMin.y, bottomMin.y);
    min.z = std::min(topMin.z, bottomMin.z);
    max.x = std::max(topMax.x, bottomMax.x);
    max.y = std::max(topMax.y, bottomMax.y);
    max.z = std::max(topMax.z, bottomMax.z);
}

float CapsuleCollider::GetVolume() const {
    float cylinderVolume = 3.14159f * m_radius * m_radius * m_height;
    float sphereVolume = (4.0f / 3.0f) * 3.14159f * m_radius * m_radius * m_radius;
    return cylinderVolume + sphereVolume;
}

Vector3 CapsuleCollider::GetCenterOfMass() const {
    return Vector3::Zero;
}

Vector3 PlaneCollider::GetSupportPoint(const Vector3& direction) const {
    const float LARGE_VALUE = 1e6f;
    if (direction.Dot(m_normal) >= 0.0f) {
        return direction * LARGE_VALUE;
    } else {
        return m_normal * m_distance;
    }
}

void PlaneCollider::GetAABB(const Vector3& position, const Quaternion& /*rotation*/, Vector3& min, Vector3& max) const {
    const float LARGE_VALUE = 1e6f;
    min = Vector3(-LARGE_VALUE, -LARGE_VALUE, -LARGE_VALUE);
    max = Vector3(LARGE_VALUE, LARGE_VALUE, LARGE_VALUE);
    
    Vector3 planePoint = position + m_normal * m_distance;
    if (m_normal.x > 0.9f) {
        min.x = planePoint.x;
    } else if (m_normal.x < -0.9f) {
        max.x = planePoint.x;
    }
    if (m_normal.y > 0.9f) {
        min.y = planePoint.y;
    } else if (m_normal.y < -0.9f) {
        max.y = planePoint.y;
    }
    if (m_normal.z > 0.9f) {
        min.z = planePoint.z;
    } else if (m_normal.z < -0.9f) {
        max.z = planePoint.z;
    }
}

float PlaneCollider::GetVolume() const {
    return 0.0f; // Planes have no volume
}

Vector3 PlaneCollider::GetCenterOfMass() const {
    return m_normal * m_distance;
}

}
