#pragma once

#include "Collider2D.h"

namespace GameEngine {
    
    class BoxCollider2D : public Collider2D {
    public:
        BoxCollider2D(const Vector2& size = Vector2::One);
        ~BoxCollider2D() override = default;
        
        // Size
        const Vector2& GetSize() const { return m_size; }
        void SetSize(const Vector2& size) { m_size = size; }
        
        Vector2 GetHalfSize() const { return m_size * 0.5f; }
        
        // Collider2D interface
        Vector2 GetMin() const override;
        Vector2 GetMax() const override;
        Vector2 GetCenter() const override;
        Vector2 GetSupport(const Vector2& direction) const override;
        float GetArea() const override;
        float CalculateInertia(float mass) const override;
        
        // Box-specific methods
        Vector2 GetVertex(int index) const; // Get corner vertex (0-3)
        
    private:
        Vector2 m_size;
    };
}
