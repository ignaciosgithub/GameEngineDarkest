#pragma once

#include "../../../Core/Math/Vector2.h"
#include "../RigidBody2D.h"

namespace GameEngine {
    class RigidBody2D;
    
    // Forward declaration - actual definition is in RigidBody2D.h
    enum class Collider2DType;
    
    class Collider2D {
    public:
        Collider2D(Collider2DType type);
        virtual ~Collider2D() = default;
        
        // Type information
        Collider2DType GetType() const { return m_type; }
        
        // Transform information
        const Vector2& GetOffset() const { return m_offset; }
        void SetOffset(const Vector2& offset) { m_offset = offset; }
        
        // Trigger settings
        bool IsTrigger() const { return m_isTrigger; }
        void SetTrigger(bool trigger) { m_isTrigger = trigger; }
        
        // Bounds calculation
        virtual Vector2 GetMin() const = 0;
        virtual Vector2 GetMax() const = 0;
        virtual Vector2 GetCenter() const = 0;
        
        // Support function for collision detection
        virtual Vector2 GetSupport(const Vector2& direction) const = 0;
        
        // Area calculation
        virtual float GetArea() const = 0;
        
        // Moment of inertia calculation
        virtual float CalculateInertia(float mass) const = 0;
        
        // Transform helpers
        Vector2 LocalToWorld(const Vector2& localPoint, const RigidBody2D* body) const;
        Vector2 WorldToLocal(const Vector2& worldPoint, const RigidBody2D* body) const;
        
    protected:
        Collider2DType m_type;
        Vector2 m_offset = Vector2::Zero;
        bool m_isTrigger = false;
    };
}
