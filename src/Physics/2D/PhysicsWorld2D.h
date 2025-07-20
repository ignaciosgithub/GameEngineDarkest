#pragma once

#include "../../Core/Math/Vector2.h"
#include "Spatial/QuadTree.h"
#include <vector>
#include <memory>

namespace GameEngine {
    class RigidBody2D;
    struct CollisionInfo2D;
    
    class PhysicsWorld2D {
    public:
        PhysicsWorld2D();
        ~PhysicsWorld2D();
        
        void Initialize();
        void Shutdown();
        
        void Update(float deltaTime);
        void FixedUpdate(float fixedDeltaTime);
        
        // Gravity (2D)
        void SetGravity(const Vector2& gravity) { m_gravity = gravity; }
        const Vector2& GetGravity() const { return m_gravity; }
        
        // Rigid body management
        void AddRigidBody(RigidBody2D* rigidBody);
        void RemoveRigidBody(RigidBody2D* rigidBody);
        const std::vector<RigidBody2D*>& GetRigidBodies() const { return m_rigidBodies; }
        
        // Collision detection
        void DetectCollisions();
        void ResolveCollisions();
        
        // Integration
        void IntegrateVelocities(float deltaTime);
        void IntegratePositions(float deltaTime);
        
        // Spatial partitioning
        void UpdateSpatialPartitioning();
        void SetUseSpatialPartitioning(bool use) { m_useSpatialPartitioning = use; }
        bool GetUseSpatialPartitioning() const { return m_useSpatialPartitioning; }
        
        // World bounds
        void SetWorldBounds(const Vector2& min, const Vector2& max);
        const Vector2& GetWorldMin() const { return m_worldMin; }
        const Vector2& GetWorldMax() const { return m_worldMax; }
        
        // Performance settings
        void SetIterations(int velocityIterations, int positionIterations) {
            m_velocityIterations = velocityIterations;
            m_positionIterations = positionIterations;
        }
        
        // Debug information
        int GetCollisionCount() const { return m_collisionCount; }
        int GetActiveBodyCount() const;
        
        // Raycasting
        bool Raycast(const Vector2& start, const Vector2& end, RigidBody2D*& hitBody, Vector2& hitPoint, Vector2& hitNormal);
        
    private:
        std::vector<RigidBody2D*> m_rigidBodies;
        std::vector<CollisionInfo2D> m_collisions;
        
        Vector2 m_gravity = Vector2(0.0f, -9.81f);
        
        // Spatial partitioning
        std::unique_ptr<QuadTree> m_quadTree;
        bool m_useSpatialPartitioning = true;
        Vector2 m_worldMin = Vector2(-100.0f, -100.0f);
        Vector2 m_worldMax = Vector2(100.0f, 100.0f);
        
        // Solver settings
        int m_velocityIterations = 8;
        int m_positionIterations = 3;
        
        // Debug info
        int m_collisionCount = 0;
        
        bool m_initialized = false;
        
        // Helper methods
        void ApplyGravity(float deltaTime);
        void BroadPhaseCollisionDetection();
        void NarrowPhaseCollisionDetection();
    };
}
