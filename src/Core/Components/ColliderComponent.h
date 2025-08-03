#pragma once

#include "../ECS/Component.h"
#include "../../Physics/Colliders/ColliderShape.h"
#include <memory>
#include <vector>

namespace GameEngine {
    class ColliderComponent : public Component<ColliderComponent> {
    public:
        ColliderComponent();
        ColliderComponent(std::shared_ptr<ColliderShape> shape);
        ~ColliderComponent() = default;
        
        // Collider shape management
        void SetColliderShape(std::shared_ptr<ColliderShape> shape);
        std::shared_ptr<ColliderShape> GetColliderShape() const { return m_colliderShape; }
        bool HasCollider() const { return m_colliderShape != nullptr; }
        
        // Convenience methods for common shapes
        void SetSphereCollider(float radius);
        void SetBoxCollider(const Vector3& halfExtents);
        void SetCapsuleCollider(float radius, float height);
        void SetPlaneCollider(const Vector3& normal, float distance);
        void SetConvexHullCollider(const std::vector<Vector3>& vertices);
        void SetTriangleMeshCollider(const std::vector<Vector3>& vertices, const std::vector<unsigned int>& indices);
        
        // Properties
        bool IsTrigger() const { return m_isTrigger; }
        void SetTrigger(bool trigger) { m_isTrigger = trigger; }
        
        // Material properties
        float GetRestitution() const { return m_restitution; }
        void SetRestitution(float restitution) { m_restitution = restitution; }
        
        float GetFriction() const { return m_friction; }
        void SetFriction(float friction) { m_friction = friction; }
        
        // Mesh-to-collider generation using vertex data
        void GenerateFromMesh(class MeshComponent* meshComponent, ColliderShapeType type = ColliderShapeType::ConvexHull);
        void ClearCollider();
        
    private:
        std::shared_ptr<ColliderShape> m_colliderShape;
        bool m_isTrigger = false;
        float m_restitution = 0.5f;
        float m_friction = 0.5f;
    };
}
