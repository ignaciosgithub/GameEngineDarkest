#include "Collider2D.h"
#include "../RigidBody2D.h"

namespace GameEngine {

Collider2D::Collider2D(Collider2DType type) : m_type(type) {
}

Vector2 Collider2D::LocalToWorld(const Vector2& localPoint, const RigidBody2D* body) const {
    if (!body) return localPoint + m_offset;
    
    Vector2 offsetPoint = localPoint + m_offset;
    return body->LocalToWorld(offsetPoint);
}

Vector2 Collider2D::WorldToLocal(const Vector2& worldPoint, const RigidBody2D* body) const {
    if (!body) return worldPoint - m_offset;
    
    Vector2 localPoint = body->WorldToLocal(worldPoint);
    return localPoint - m_offset;
}

}
