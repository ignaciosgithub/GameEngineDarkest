#include "RigidBody2D.h"
#include "Colliders/Collider2D.h"
#include "../Materials/PhysicsMaterial.h"
#include "../../Core/Logging/Logger.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GameEngine {

const float RigidBody2D::SLEEP_TIME_THRESHOLD = 1.0f;

RigidBody2D::RigidBody2D() {
    m_material = PhysicsMaterial::GetDefault();
    Logger::Debug("RigidBody2D created");
}

RigidBody2D::~RigidBody2D() {
    Logger::Debug("RigidBody2D destroyed");
}

void RigidBody2D::AddForce(const Vector2& force) {
    if (!IsDynamic()) return;
    m_force = m_force + force;
    WakeUp();
}

void RigidBody2D::AddForceAtPosition(const Vector2& force, const Vector2& position) {
    if (!IsDynamic()) return;
    
    AddForce(force);
    
    Vector2 r = position - m_position;
    float torque = r.x * force.y - r.y * force.x; // 2D cross product
    AddTorque(torque);
}

void RigidBody2D::AddTorque(float torque) {
    if (!IsDynamic() || m_freezeRotation) return;
    m_torque += torque;
    WakeUp();
}

void RigidBody2D::AddImpulse(const Vector2& impulse) {
    if (!IsDynamic()) return;
    m_velocity = m_velocity + impulse * GetInverseMass();
    WakeUp();
}

void RigidBody2D::AddImpulseAtPosition(const Vector2& impulse, const Vector2& position) {
    if (!IsDynamic()) return;
    
    AddImpulse(impulse);
    
    Vector2 r = position - m_position;
    float angularImpulse = r.x * impulse.y - r.y * impulse.x;
    
    if (!m_freezeRotation) {
        m_angularVelocity += angularImpulse * GetInverseInertia();
    }
}

float RigidBody2D::GetRestitution() const {
    return m_material ? m_material->GetRestitution() : m_restitution;
}

void RigidBody2D::SetRestitution(float restitution) {
    m_restitution = restitution;
    if (m_material) {
        m_material->SetRestitution(restitution);
    }
}

float RigidBody2D::GetFriction() const {
    return m_material ? m_material->GetDynamicFriction() : m_friction;
}

void RigidBody2D::SetFriction(float friction) {
    m_friction = friction;
    if (m_material) {
        m_material->SetDynamicFriction(friction);
    }
}

void RigidBody2D::IntegrateVelocity(float deltaTime) {
    if (!IsDynamic() || m_sleeping) return;
    
    Vector2 acceleration = m_force * GetInverseMass();
    m_velocity = m_velocity + acceleration * deltaTime;
    
    float damping = 1.0f - m_damping;
    m_velocity = m_velocity * damping;
    
    if (!m_freezeRotation) {
        float angularAcceleration = m_torque * GetInverseInertia();
        m_angularVelocity += angularAcceleration * deltaTime;
        
        float angularDampingFactor = 1.0f - m_angularDamping;
        m_angularVelocity *= angularDampingFactor;
    }
    
    float speed = m_velocity.Length() + std::abs(m_angularVelocity);
    if (speed < m_sleepThreshold) {
        m_sleepTimer += deltaTime;
        if (m_sleepTimer > SLEEP_TIME_THRESHOLD) {
            SetSleeping(true);
        }
    } else {
        m_sleepTimer = 0.0f;
    }
}

void RigidBody2D::IntegratePosition(float deltaTime) {
    if (!IsDynamic() || m_sleeping) return;
    
    Vector2 constrainedVelocity = m_velocity;
    if (m_freezePositionX) constrainedVelocity.x = 0.0f;
    if (m_freezePositionY) constrainedVelocity.y = 0.0f;
    
    m_position = m_position + constrainedVelocity * deltaTime;
    
    if (!m_freezeRotation) {
        m_rotation += m_angularVelocity * deltaTime;
        
        while (m_rotation > static_cast<float>(M_PI)) m_rotation -= 2.0f * static_cast<float>(M_PI);
        while (m_rotation < -static_cast<float>(M_PI)) m_rotation += 2.0f * static_cast<float>(M_PI);
    }
}

Vector2 RigidBody2D::GetPointVelocity(const Vector2& worldPoint) const {
    Vector2 r = worldPoint - m_position;
    Vector2 angularComponent(-m_angularVelocity * r.y, m_angularVelocity * r.x);
    return m_velocity + angularComponent;
}

Vector2 RigidBody2D::LocalToWorld(const Vector2& localPoint) const {
    float cos_r = std::cos(m_rotation);
    float sin_r = std::sin(m_rotation);
    
    Vector2 rotated(
        localPoint.x * cos_r - localPoint.y * sin_r,
        localPoint.x * sin_r + localPoint.y * cos_r
    );
    
    return m_position + rotated;
}

Vector2 RigidBody2D::WorldToLocal(const Vector2& worldPoint) const {
    Vector2 relative = worldPoint - m_position;
    float cos_r = std::cos(-m_rotation); // Inverse rotation
    float sin_r = std::sin(-m_rotation);
    
    return Vector2(
        relative.x * cos_r - relative.y * sin_r,
        relative.x * sin_r + relative.y * cos_r
    );
}

Vector2 RigidBody2D::LocalDirectionToWorld(const Vector2& localDirection) const {
    float cos_r = std::cos(m_rotation);
    float sin_r = std::sin(m_rotation);
    
    return Vector2(
        localDirection.x * cos_r - localDirection.y * sin_r,
        localDirection.x * sin_r + localDirection.y * cos_r
    );
}

Vector2 RigidBody2D::WorldDirectionToLocal(const Vector2& worldDirection) const {
    float cos_r = std::cos(-m_rotation); // Inverse rotation
    float sin_r = std::sin(-m_rotation);
    
    return Vector2(
        worldDirection.x * cos_r - worldDirection.y * sin_r,
        worldDirection.x * sin_r + worldDirection.y * cos_r
    );
}

}
