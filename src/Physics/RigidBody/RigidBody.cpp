#include "RigidBody.h"
#include "../Materials/PhysicsMaterial.h"
#include "../../Core/Components/ColliderComponent.h"
#include "../../Core/Components/TransformComponent.h"
#include "../../Core/Logging/Logger.h"
#include "../Colliders/ColliderShape.h"
#include <algorithm>
#include <memory>
#include <cmath>

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
        if (m_inertiaDirty) RecomputeBodyInertia();
        m_angularVelocity = m_angularVelocity + ApplyInvInertiaWorld(angularImpulse);
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
    
    float linDamp = std::exp(-m_damping * deltaTime);
    m_velocity = m_velocity * linDamp;
    
    if (!m_freezeRotation) {
        if (m_inertiaDirty) RecomputeBodyInertia();
        Vector3 angAcc = ApplyInvInertiaWorld(m_torque);
        m_angularVelocity = m_angularVelocity + angAcc * deltaTime;
        float angDamp = std::exp(-m_angularDamping * deltaTime);
        m_angularVelocity = m_angularVelocity * angDamp;
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
Vector3 RigidBody::ApplyInvInertiaWorld(const Vector3& angularImpulse) const {
    Quaternion q = m_rotation;
    Quaternion qInv = q.Inverse();
    Vector3 L_body = qInv.RotateVector(angularImpulse);
    Vector3 omega_body(L_body.x * m_invInertiaDiag.x,
                       L_body.y * m_invInertiaDiag.y,
                       L_body.z * m_invInertiaDiag.z);
    return q.RotateVector(omega_body);
}
Vector3 RigidBody::InvInertiaWorldMultiply(const Vector3& v) const {
    if (m_inertiaDirty) {
        const_cast<RigidBody*>(this)->RecomputeBodyInertia();
    }
    return ApplyInvInertiaWorld(v);
}

void RigidBody::RecomputeBodyInertia() {
    if (!IsDynamic()) {
        m_inertiaDiag = Vector3(1.0f, 1.0f, 1.0f);
        m_invInertiaDiag = Vector3(0.0f, 0.0f, 0.0f);
        m_inertiaDirty = false;
        return;
    }
    float m = GetMass();
    if (m <= 0.0f) {
        m_inertiaDiag = Vector3(1.0f, 1.0f, 1.0f);
        m_invInertiaDiag = Vector3(0.0f, 0.0f, 0.0f);
        m_inertiaDirty = false;
        return;
    }
    Vector3 scale(1.0f, 1.0f, 1.0f);
    if (m_transformComponent) {
        scale = m_transformComponent->transform.GetWorldScale();
    }
    Vector3 I;
    if (m_colliderComponent && m_colliderComponent->HasCollider()) {
        auto shape = m_colliderComponent->GetColliderShape();
        if (shape->GetType() == ColliderShapeType::Box) {
            auto box = std::static_pointer_cast<BoxCollider>(shape);
            Vector3 he = box->GetHalfExtents();
            Vector3 e(2.0f * he.x * scale.x, 2.0f * he.y * scale.y, 2.0f * he.z * scale.z);
            I.x = (m / 12.0f) * (e.y * e.y + e.z * e.z);
            I.y = (m / 12.0f) * (e.x * e.x + e.z * e.z);
            I.z = (m / 12.0f) * (e.x * e.x + e.y * e.y);
        } else if (shape->GetType() == ColliderShapeType::Sphere) {
            auto sph = std::static_pointer_cast<SphereCollider>(shape);
            float r = sph->GetRadius();
            float s = (scale.x + scale.y + scale.z) / 3.0f;
            float Iall = (2.0f / 5.0f) * m * (r * s) * (r * s);
            I = Vector3(Iall, Iall, Iall);
        } else if (shape->GetType() == ColliderShapeType::Capsule) {
            auto cap = std::static_pointer_cast<CapsuleCollider>(shape);
            float r = cap->GetRadius() * 0.5f * (scale.x + scale.z);
            float h = cap->GetHeight() * scale.y;
            float Iyy = 0.5f * m * r * r;
            float Ixx = (1.0f / 12.0f) * m * (3.0f * r * r + h * h);
            float Izz = Ixx;
            I = Vector3(Ixx, Iyy, Izz);
        } else {
            I = Vector3(m / 6.0f, m / 6.0f, m / 6.0f);
        }
    } else {
        I = Vector3(m / 6.0f, m / 6.0f, m / 6.0f);
    }
    m_inertiaDiag = I;
    m_invInertiaDiag = Vector3(
        I.x > 1e-8f ? 1.0f / I.x : 0.0f,
        I.y > 1e-8f ? 1.0f / I.y : 0.0f,
        I.z > 1e-8f ? 1.0f / I.z : 0.0f
    );
    m_inertiaDirty = false;
}


void RigidBody::SetColliderComponent(ColliderComponent* colliderComponent) {
    m_colliderComponent = colliderComponent;
    m_inertiaDirty = true;
}

void RigidBody::SetTransformComponent(class TransformComponent* transformComponent) {
    m_transformComponent = transformComponent;
    m_inertiaDirty = true;
}

}
