#pragma once

#include "../../Core/Math/Vector3.h"
#include "../../Core/Math/Quaternion.h"
#include <memory>

namespace GameEngine {
    enum class ColliderShapeType {
        None = 0,
        Sphere,
        Box,
        Capsule,
        Plane,
        ConvexHull,
        TriangleMesh
    };
    
    class ColliderShape {
    public:
        ColliderShape(ColliderShapeType type) : m_type(type) {}
        virtual ~ColliderShape() = default;
        
        ColliderShapeType GetType() const { return m_type; }
        
        // Virtual methods for collision detection
        virtual Vector3 GetSupportPoint(const Vector3& direction) const = 0;
        virtual void GetAABB(const Vector3& position, const Quaternion& rotation, Vector3& min, Vector3& max) const = 0;
        virtual float GetVolume() const = 0;
        virtual Vector3 GetCenterOfMass() const = 0;
        
    protected:
        ColliderShapeType m_type;
    };
    
    class SphereCollider : public ColliderShape {
    public:
        SphereCollider(float radius) : ColliderShape(ColliderShapeType::Sphere), m_radius(radius) {}
        
        float GetRadius() const { return m_radius; }
        void SetRadius(float radius) { m_radius = radius; }
        
        Vector3 GetSupportPoint(const Vector3& direction) const override;
        void GetAABB(const Vector3& position, const Quaternion& rotation, Vector3& min, Vector3& max) const override;
        float GetVolume() const override;
        Vector3 GetCenterOfMass() const override;
        
    private:
        float m_radius;
    };
    
    class BoxCollider : public ColliderShape {
    public:
        BoxCollider(const Vector3& halfExtents) : ColliderShape(ColliderShapeType::Box), m_halfExtents(halfExtents) {}
        
        const Vector3& GetHalfExtents() const { return m_halfExtents; }
        void SetHalfExtents(const Vector3& halfExtents) { m_halfExtents = halfExtents; }
        
        Vector3 GetSupportPoint(const Vector3& direction) const override;
        void GetAABB(const Vector3& position, const Quaternion& rotation, Vector3& min, Vector3& max) const override;
        float GetVolume() const override;
        Vector3 GetCenterOfMass() const override;
        
    private:
        Vector3 m_halfExtents;
    };
    
    class CapsuleCollider : public ColliderShape {
    public:
        CapsuleCollider(float radius, float height) 
            : ColliderShape(ColliderShapeType::Capsule), m_radius(radius), m_height(height) {}
        
        float GetRadius() const { return m_radius; }
        float GetHeight() const { return m_height; }
        void SetRadius(float radius) { m_radius = radius; }
        void SetHeight(float height) { m_height = height; }
        
        Vector3 GetSupportPoint(const Vector3& direction) const override;
        void GetAABB(const Vector3& position, const Quaternion& rotation, Vector3& min, Vector3& max) const override;
        float GetVolume() const override;
        Vector3 GetCenterOfMass() const override;
        
    private:
        float m_radius;
        float m_height;
    };
    
    class PlaneCollider : public ColliderShape {
    public:
        PlaneCollider(const Vector3& normal, float distance) 
            : ColliderShape(ColliderShapeType::Plane), m_normal(normal), m_distance(distance) {}
        
        const Vector3& GetNormal() const { return m_normal; }
        float GetDistance() const { return m_distance; }
        void SetNormal(const Vector3& normal) { m_normal = normal; }
        void SetDistance(float distance) { m_distance = distance; }
        
        Vector3 GetSupportPoint(const Vector3& direction) const override;
        void GetAABB(const Vector3& position, const Quaternion& rotation, Vector3& min, Vector3& max) const override;
        float GetVolume() const override;
        Vector3 GetCenterOfMass() const override;
        
    private:
        Vector3 m_normal;
        float m_distance;
    };
}
