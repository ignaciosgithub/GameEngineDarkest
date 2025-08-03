#pragma once

#include "../../Core/Math/Vector3.h"
#include "../../Core/Math/Quaternion.h"

namespace GameEngine {
    class PhysicsMaterial;
    
    enum class ColliderType {
        None = 0,
        Sphere,
        Box,
        Plane,
        Capsule,
        ConvexHull,
        TriangleMesh
    };
    
    enum class RigidBodyType {
        Static = 0,    // Immovable, infinite mass
        Kinematic,     // Movable by script, not affected by forces
        Dynamic        // Affected by forces and collisions
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
        const Vector3& GetTorque() const { return m_torque; }
        void AddForce(const Vector3& force);
        void AddForceAtPosition(const Vector3& force, const Vector3& position);
        void AddTorque(const Vector3& torque);
        void AddImpulse(const Vector3& impulse);
        void AddImpulseAtPosition(const Vector3& impulse, const Vector3& position);
        void ClearForces() { m_force = Vector3::Zero; m_torque = Vector3::Zero; }
        
        // Mass and inertia
        float GetMass() const { return m_mass; }
        void SetMass(float mass) { m_mass = mass; m_invMass = (mass > 0.0f) ? 1.0f / mass : 0.0f; }
        float GetInverseMass() const { return m_invMass; }
        
        // Body type
        RigidBodyType GetBodyType() const { return m_bodyType; }
        void SetBodyType(RigidBodyType type) { m_bodyType = type; }
        
        // Physics material
        PhysicsMaterial* GetMaterial() const { return m_material; }
        void SetMaterial(PhysicsMaterial* material) { m_material = material; }
        
        // Legacy material properties (for backward compatibility)
        float GetRestitution() const;
        void SetRestitution(float restitution);
        
        float GetFriction() const;
        void SetFriction(float friction);
        
        float GetDamping() const { return m_damping; }
        void SetDamping(float damping) { m_damping = damping; }
        
        // Body state checks
        bool IsStatic() const { return m_bodyType == RigidBodyType::Static; }
        bool IsKinematic() const { return m_bodyType == RigidBodyType::Kinematic; }
        bool IsDynamic() const { return m_bodyType == RigidBodyType::Dynamic; }
        
        // Sleeping/activation
        bool IsSleeping() const { return m_sleeping; }
        void SetSleeping(bool sleeping) { m_sleeping = sleeping; }
        void WakeUp() { m_sleeping = false; }
        
        // Constraints
        void SetFreezeRotation(bool freeze) { m_freezeRotation = freeze; }
        bool IsFreezeRotation() const { return m_freezeRotation; }
        
        void SetFreezePosition(const Vector3& freeze) { m_freezePosition = freeze; }
        const Vector3& GetFreezePosition() const { return m_freezePosition; }
        
        // Collider component integration
        void SetColliderComponent(class ColliderComponent* colliderComponent);
        class ColliderComponent* GetColliderComponent() const { return m_colliderComponent; }
        
        // Transform component integration
        void SetTransformComponent(class TransformComponent* transformComponent);
        class TransformComponent* GetTransformComponent() const { return m_transformComponent; }
        
        // Integration
        void IntegrateVelocity(float deltaTime);
        void IntegratePosition(float deltaTime);
        
        // Utility
        Vector3 GetPointVelocity(const Vector3& worldPoint) const;
        
    private:
        // Transform
        Vector3 m_position = Vector3::Zero;
        Quaternion m_rotation = Quaternion::Identity();
        
        // Motion
        Vector3 m_velocity = Vector3::Zero;
        Vector3 m_angularVelocity = Vector3::Zero;
        Vector3 m_force = Vector3::Zero;
        Vector3 m_torque = Vector3::Zero;
        
        // Physical properties
        float m_mass = 1.0f;
        float m_invMass = 1.0f;
        RigidBodyType m_bodyType = RigidBodyType::Dynamic;
        float m_damping = 0.01f;
        
        // Physics material
        PhysicsMaterial* m_material = nullptr;
        
        // Legacy properties
        float m_restitution = 0.5f;
        float m_friction = 0.5f;
        
        // State
        bool m_sleeping = false;
        bool m_freezeRotation = false;
        Vector3 m_freezePosition = Vector3::Zero; // 0 = free, 1 = frozen
        
        // Sleep threshold
        float m_sleepThreshold = 0.1f;
        float m_sleepTimer = 0.0f;
        static const float SLEEP_TIME_THRESHOLD;
        
        // Collider component reference
        class ColliderComponent* m_colliderComponent = nullptr;
        
        // Transform component reference for coordinate transformation
        class TransformComponent* m_transformComponent = nullptr;
    };
}
