#include "CircleCollider2D.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GameEngine {

CircleCollider2D::CircleCollider2D(float radius) 
    : Collider2D(Collider2DType::Circle), m_radius(radius) {
}

Vector2 CircleCollider2D::GetMin() const {
    Vector2 center = GetCenter();
    return Vector2(center.x - m_radius, center.y - m_radius);
}

Vector2 CircleCollider2D::GetMax() const {
    Vector2 center = GetCenter();
    return Vector2(center.x + m_radius, center.y + m_radius);
}

Vector2 CircleCollider2D::GetCenter() const {
    return m_offset;
}

Vector2 CircleCollider2D::GetSupport(const Vector2& direction) const {
    Vector2 normalizedDir = direction.Normalized();
    return GetCenter() + normalizedDir * m_radius;
}

float CircleCollider2D::GetArea() const {
    return M_PI * m_radius * m_radius;
}

float CircleCollider2D::CalculateInertia(float mass) const {
    return 0.5f * mass * m_radius * m_radius;
}

}
