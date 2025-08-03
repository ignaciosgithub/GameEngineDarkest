#include "RigidBody.h"
#include "../Materials/PhysicsMaterial.h"
#include "../../Core/Components/ColliderComponent.h"
#include "../../Core/Logging/Logger.h"
#include <algorithm>

namespace GameEngine {

const float RigidBody::SLEEP_TIME_THRESHOLD = 1.0f;

RigidBody::RigidBody() {
    m_material = PhysicsMaterial::GetDefault();
    Logger::Debug("RigidBody created");
}

RigidBody::~RigidBody() {
    Logger::Debug("RigidBody destroyed");
}

void RigidBody::AddForce(const Vector3& force) {
    if (!IsDynamic()) return;
    m_force = m_force + force;
    WakeUp();
}

void RigidBody::AddForceAtPosition(const Vector3& force, const Vector3& position) {
    if (!IsDynamic()) return;
    
    AddForce(force);
    
    Vector3 r = position - m_position;
    Vector3 torque = r.Cross(force);
    AddTorque(torque);
}

void RigidBody::AddTorque(const Vector3& torque) {
    if (!IsDynamic() || m_freezeRotation) return;
    m_torque = m_torque + torque;
    WakeUp();
}

void RigidBody::AddImpulse(const Vector3& impulse) {
    if (!IsDynamic()) return;
    m_velocity = m_velocity + impulse * GetInverseMass();
    WakeUp();
}

void RigidBody::AddImpulseAtPosition(const Vector3& impulse, const Vector3& position) {
    if (!IsDynamic()) return;
    
    AddImpulse(impulse);
    
    Vector3 r = position - m_position;
    Vector3 angularImpulse = r.Cross(impulse);
    
    if (!m_freezeRotation) {
        m_angularVelocity = m_angularVelocity + angularImpulse * GetInverseMass();
    }
}

float RigidBody::GetRestitution() const {
    return m_material ? m_material->GetRestitution() : m_restitution;
}

void RigidBody::SetRestitution(float restitution) {
    m_restitution = restitution;
    if (m_material) {
        m_material->SetRestitution(restitution);
    }
}

float RigidBody::GetFriction() const {
    return m_material ? m_material->GetDynamicFriction() : m_friction;
}

void RigidBody::SetFriction(float friction) {
    m_friction = friction;
    if (m_material) {
        m_material->SetDynamicFriction(friction);
    }
}

void RigidBody::IntegrateVelocity(float deltaTime) {
    if (!IsDynamic() || m_sleeping) return;
    
    Vector3 acceleration = m_force * GetInverseMass();
    m_velocity = m_velocity + acceleration * deltaTime;
    
    float damping = 0.99f;
    m_velocity = m_velocity * damping;
    
    if (!m_freezeRotation) {
        m_angularVelocity = m_angularVelocity + m_torque * deltaTime;
        m_angularVelocity = m_angularVelocity * damping;
    }
    
    float speed = m_velocity.Length() + m_angularVelocity.Length();
    if (speed < m_sleepThreshold) {
        m_sleepTimer += deltaTime;
        if (m_sleepTimer > SLEEP_TIME_THRESHOLD) {
            SetSleeping(true);
        }
    } else {
        m_sleepTimer = 0.0f;
    }
}

void RigidBody::IntegratePosition(float deltaTime) {
    if (!IsDynamic() || m_sleeping) return;
    
    Vector3 constrainedVelocity = m_velocity;
    if (m_freezePosition.x > 0.5f) constrainedVelocity.x = 0.0f;
    if (m_freezePosition.y > 0.5f) constrainedVelocity.y = 0.0f;
    if (m_freezePosition.z > 0.5f) constrainedVelocity.z = 0.0f;
    
    m_position = m_position + constrainedVelocity * deltaTime;
    
    if (!m_freezeRotation && m_angularVelocity.Length() > 0.0f) {
        float angle = m_angularVelocity.Length() * deltaTime;
        Vector3 axis = m_angularVelocity.Normalized();
        Quaternion deltaRotation = Quaternion::FromAxisAngle(axis, angle);
        m_rotation = deltaRotation * m_rotation;
        m_rotation = m_rotation.Normalized();
    }
}

Vector3 RigidBody::GetPointVelocity(const Vector3& worldPoint) const {
    Vector3 r = worldPoint - m_position;
    return m_velocity + m_angularVelocity.Cross(r);
}

void RigidBody::SetColliderComponent(ColliderComponent* colliderComponent) {
    m_colliderComponent = colliderComponent;
}

}
