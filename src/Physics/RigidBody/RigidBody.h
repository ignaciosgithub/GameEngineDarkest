#pragma once

#include "../../Core/Math/Vector3.h"
#include "../../Core/Math/Quaternion.h"

namespace GameEngine {
    enum class ColliderType {
        None = 0,
        Sphere,
        Box,
        Plane
    };
    
    class RigidBody {
    public:
        RigidBody();
        ~RigidBody();
        
        // Position and rotation
        const Vector3& GetPosition() const { return m_position; }
        void SetPosition(const Vector3& position) { m_position = position; }
        
        const Quaternion& GetRotation() const { return m_rotation; }
        void SetRotation(const Quaternion& rotation) { m_rotation = rotation; }
        
        // Velocity and angular velocity
        const Vector3& GetVelocity() const { return m_velocity; }
        void SetVelocity(const Vector3& velocity) { m_velocity = velocity; }
        
        const Vector3& GetAngularVelocity() const { return m_angularVelocity; }
        void SetAngularVelocity(const Vector3& angularVelocity) { m_angularVelocity = angularVelocity; }
        
        // Forces
        const Vector3& GetForce() const { return m_force; }
        void AddForce(const Vector3& force) { m_force += force; }
        void ClearForces() { m_force = Vector3::Zero; }
        
        // Mass and inertia
        float GetMass() const { return m_mass; }
        void SetMass(float mass) { m_mass = mass; m_invMass = (mass > 0.0f) ? 1.0f / mass : 0.0f; }
        float GetInverseMass() const { return m_invMass; }
        
        // Material properties
        float GetRestitution() const { return m_restitution; }
        void SetRestitution(float restitution) { m_restitution = restitution; }
        
        float GetFriction() const { return m_friction; }
        void SetFriction(float friction) { m_friction = friction; }
        
        float GetDamping() const { return m_damping; }
        void SetDamping(float damping) { m_damping = damping; }
        
        // Static/kinematic flags
        bool IsStatic() const { return m_isStatic; }
        void SetStatic(bool isStatic) { m_isStatic = isStatic; }
        
        bool IsKinematic() const { return m_isKinematic; }
        void SetKinematic(bool isKinematic) { m_isKinematic = isKinematic; }
        
        // Collider
        ColliderType GetColliderType() const { return m_colliderType; }
        void SetColliderType(ColliderType type) { m_colliderType = type; }
        
        const Vector3& GetColliderSize() const { return m_colliderSize; }
        void SetColliderSize(const Vector3& size) { m_colliderSize = size; }
        
    private:
        // Transform
        Vector3 m_position = Vector3::Zero;
        Quaternion m_rotation = Quaternion::Identity();
        
        // Motion
        Vector3 m_velocity = Vector3::Zero;
        Vector3 m_angularVelocity = Vector3::Zero;
        Vector3 m_force = Vector3::Zero;
        
        // Physical properties
        float m_mass = 1.0f;
        float m_invMass = 1.0f;
        float m_restitution = 0.5f;
        float m_friction = 0.5f;
        float m_damping = 0.01f;
        
        // Flags
        bool m_isStatic = false;
        bool m_isKinematic = false;
        
        // Collider
        ColliderType m_colliderType = ColliderType::None;
        Vector3 m_colliderSize = Vector3::One;
    };
}
