#pragma once

#include "Collider2D.h"

namespace GameEngine {
    
    class CircleCollider2D : public Collider2D {
    public:
        CircleCollider2D(float radius = 0.5f);
        ~CircleCollider2D() override = default;
        
        // Radius
        float GetRadius() const { return m_radius; }
        void SetRadius(float radius) { m_radius = radius; }
        
        // Collider2D interface
        Vector2 GetMin() const override;
        Vector2 GetMax() const override;
        Vector2 GetCenter() const override;
        Vector2 GetSupport(const Vector2& direction) const override;
        float GetArea() const override;
        float CalculateInertia(float mass) const override;
        
    private:
        float m_radius;
    };
}
