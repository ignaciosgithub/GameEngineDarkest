#pragma once

#include "../../Core/Math/Vector2.h"

namespace GameEngine {
    class PhysicsMaterial;
    
    enum class Collider2DType {
        None = 0,
        Circle,
        Box,
        Polygon,
        Edge
    };
    
    enum class RigidBody2DType {
        Static = 0,    // Immovable, infinite mass
        Kinematic,     // Movable by script, not affected by forces
        Dynamic        // Affected by forces and collisions
    };
    
    class RigidBody2D {
    public:
        RigidBody2D();
        ~RigidBody2D();
        
        // Position and rotation (2D)
        const Vector2& GetPosition() const { return m_position; }
        void SetPosition(const Vector2& position) { m_position = position; }
        
        float GetRotation() const { return m_rotation; }
        void SetRotation(float rotation) { m_rotation = rotation; }
        
        // Velocity and angular velocity (2D)
        const Vector2& GetVelocity() const { return m_velocity; }
        void SetVelocity(const Vector2& velocity) { m_velocity = velocity; }
        
        float GetAngularVelocity() const { return m_angularVelocity; }
        void SetAngularVelocity(float angularVelocity) { m_angularVelocity = angularVelocity; }
        
        // Forces (2D)
        const Vector2& GetForce() const { return m_force; }
        float GetTorque() const { return m_torque; }
        void AddForce(const Vector2& force);
        void AddForceAtPosition(const Vector2& force, const Vector2& position);
        void AddTorque(float torque);
        void AddImpulse(const Vector2& impulse);
        void AddImpulseAtPosition(const Vector2& impulse, const Vector2& position);
        void ClearForces() { m_force = Vector2::Zero; m_torque = 0.0f; }
        
        // Mass and inertia (2D)
        float GetMass() const { return m_mass; }
        void SetMass(float mass) { m_mass = mass; m_invMass = (mass > 0.0f) ? 1.0f / mass : 0.0f; }
        float GetInverseMass() const { return m_invMass; }
        
        float GetInertia() const { return m_inertia; }
        void SetInertia(float inertia) { m_inertia = inertia; m_invInertia = (inertia > 0.0f) ? 1.0f / inertia : 0.0f; }
        float GetInverseInertia() const { return m_invInertia; }
        
        // Body type
        RigidBody2DType GetBodyType() const { return m_bodyType; }
        void SetBodyType(RigidBody2DType type) { m_bodyType = type; }
        
        // Physics material
        PhysicsMaterial* GetMaterial() const { return m_material; }
        void SetMaterial(PhysicsMaterial* material) { m_material = material; }
        
        // Material properties (for backward compatibility)
        float GetRestitution() const;
        void SetRestitution(float restitution);
        
        float GetFriction() const;
        void SetFriction(float friction);
        
        float GetDamping() const { return m_damping; }
        void SetDamping(float damping) { m_damping = damping; }
        
        float GetAngularDamping() const { return m_angularDamping; }
        void SetAngularDamping(float damping) { m_angularDamping = damping; }
        
        // Body state checks
        bool IsStatic() const { return m_bodyType == RigidBody2DType::Static; }
        bool IsKinematic() const { return m_bodyType == RigidBody2DType::Kinematic; }
        bool IsDynamic() const { return m_bodyType == RigidBody2DType::Dynamic; }
        
        // Sleeping/activation
        bool IsSleeping() const { return m_sleeping; }
        void SetSleeping(bool sleeping) { m_sleeping = sleeping; }
        void WakeUp() { m_sleeping = false; }
        
        // Constraints (2D)
        void SetFreezeRotation(bool freeze) { m_freezeRotation = freeze; }
        bool IsFreezeRotation() const { return m_freezeRotation; }
        
        void SetFreezePositionX(bool freeze) { m_freezePositionX = freeze; }
        bool IsFreezePositionX() const { return m_freezePositionX; }
        
        void SetFreezePositionY(bool freeze) { m_freezePositionY = freeze; }
        bool IsFreezePositionY() const { return m_freezePositionY; }
        
        // Collider (2D)
        Collider2DType GetColliderType() const { return m_colliderType; }
        void SetColliderType(Collider2DType type) { m_colliderType = type; }
        
        const Vector2& GetColliderSize() const { return m_colliderSize; }
        void SetColliderSize(const Vector2& size) { m_colliderSize = size; }
        
        float GetColliderRadius() const { return m_colliderRadius; }
        void SetColliderRadius(float radius) { m_colliderRadius = radius; }
        
        // Integration (2D)
        void IntegrateVelocity(float deltaTime);
        void IntegratePosition(float deltaTime);
        
        // Utility (2D)
        Vector2 GetPointVelocity(const Vector2& worldPoint) const;
        
        // Transform helpers
        Vector2 LocalToWorld(const Vector2& localPoint) const;
        Vector2 WorldToLocal(const Vector2& worldPoint) const;
        Vector2 LocalDirectionToWorld(const Vector2& localDirection) const;
        Vector2 WorldDirectionToLocal(const Vector2& worldDirection) const;
        
    private:
        // Transform (2D)
        Vector2 m_position = Vector2::Zero;
        float m_rotation = 0.0f; // Angle in radians
        
        // Motion (2D)
        Vector2 m_velocity = Vector2::Zero;
        float m_angularVelocity = 0.0f; // Scalar angular velocity
        Vector2 m_force = Vector2::Zero;
        float m_torque = 0.0f;
        
        // Physical properties (2D)
        float m_mass = 1.0f;
        float m_invMass = 1.0f;
        float m_inertia = 1.0f; // Moment of inertia (scalar for 2D)
        float m_invInertia = 1.0f;
        RigidBody2DType m_bodyType = RigidBody2DType::Dynamic;
        float m_damping = 0.01f;
        float m_angularDamping = 0.01f;
        
        // Physics material
        PhysicsMaterial* m_material = nullptr;
        
        // Legacy properties
        float m_restitution = 0.5f;
        float m_friction = 0.5f;
        
        // State
        bool m_sleeping = false;
        bool m_freezeRotation = false;
        bool m_freezePositionX = false;
        bool m_freezePositionY = false;
        
        // Sleep threshold
        float m_sleepThreshold = 0.1f;
        float m_sleepTimer = 0.0f;
        static const float SLEEP_TIME_THRESHOLD;
        
        // Collider (2D)
        Collider2DType m_colliderType = Collider2DType::None;
        Vector2 m_colliderSize = Vector2::One; // For box colliders
        float m_colliderRadius = 0.5f; // For circle colliders
    };
}
