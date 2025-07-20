#include "BoxCollider2D.h"
#include <algorithm>

namespace GameEngine {

BoxCollider2D::BoxCollider2D(const Vector2& size) 
    : Collider2D(Collider2DType::Box), m_size(size) {
}

Vector2 BoxCollider2D::GetMin() const {
    Vector2 center = GetCenter();
    Vector2 halfSize = GetHalfSize();
    return Vector2(center.x - halfSize.x, center.y - halfSize.y);
}

Vector2 BoxCollider2D::GetMax() const {
    Vector2 center = GetCenter();
    Vector2 halfSize = GetHalfSize();
    return Vector2(center.x + halfSize.x, center.y + halfSize.y);
}

Vector2 BoxCollider2D::GetCenter() const {
    return m_offset;
}

Vector2 BoxCollider2D::GetSupport(const Vector2& direction) const {
    Vector2 center = GetCenter();
    Vector2 halfSize = GetHalfSize();
    
    Vector2 support = center;
    support.x += (direction.x >= 0.0f) ? halfSize.x : -halfSize.x;
    support.y += (direction.y >= 0.0f) ? halfSize.y : -halfSize.y;
    
    return support;
}

float BoxCollider2D::GetArea() const {
    return m_size.x * m_size.y;
}

float BoxCollider2D::CalculateInertia(float mass) const {
    return (mass / 12.0f) * (m_size.x * m_size.x + m_size.y * m_size.y);
}

Vector2 BoxCollider2D::GetVertex(int index) const {
    Vector2 center = GetCenter();
    Vector2 halfSize = GetHalfSize();
    
    switch (index % 4) {
        case 0: return Vector2(center.x - halfSize.x, center.y - halfSize.y); // Bottom-left
        case 1: return Vector2(center.x + halfSize.x, center.y - halfSize.y); // Bottom-right
        case 2: return Vector2(center.x + halfSize.x, center.y + halfSize.y); // Top-right
        case 3: return Vector2(center.x - halfSize.x, center.y + halfSize.y); // Top-left
        default: return center;
    }
}

}
