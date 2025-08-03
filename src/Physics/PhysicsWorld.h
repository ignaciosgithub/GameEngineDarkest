#pragma once

#include <vector>
#include <memory>
#include "../Core/Math/Vector3.h"
#include "Collision/CollisionDetection.h"

namespace GameEngine {
    class RigidBody;
    class ColliderComponent;
    class Entity;
    class World;
    class Octree;
    class PhysicsWorld2D;
    
    class PhysicsWorld {
    public:
        PhysicsWorld();
        ~PhysicsWorld();
        
        void Initialize();
        void Shutdown();
        
        void Update(float deltaTime);
        void FixedUpdate(float fixedDeltaTime);
        
        // Gravity
        void SetGravity(const Vector3& gravity) { m_gravity = gravity; }
        const Vector3& GetGravity() const { return m_gravity; }
        
        // Rigid body management
        void AddRigidBody(RigidBody* rigidBody);
        void RemoveRigidBody(RigidBody* rigidBody);
        
        // Static collider management
        void AddStaticCollider(ColliderComponent* collider);
        void RemoveStaticCollider(ColliderComponent* collider);
        
        // Collision detection
        void DetectCollisions();
        void ResolveCollisions();
        
        // Integration
        void IntegrateVelocities(float deltaTime);
        void IntegratePositions(float deltaTime);
        
        // Spatial partitioning
        void UpdateSpatialPartitioning();
        
        // Performance settings
        void SetUseSpatialPartitioning(bool use) { m_useSpatialPartitioning = use; }
        bool GetUseSpatialPartitioning() const { return m_useSpatialPartitioning; }
        
        // Physics timestep settings
        void SetMaxPhysicsStepsPerFrame(int maxSteps) { m_maxPhysicsStepsPerFrame = maxSteps; }
        int GetMaxPhysicsStepsPerFrame() const { return m_maxPhysicsStepsPerFrame; }
        
        // 2D Physics integration
        PhysicsWorld2D* GetPhysicsWorld2D() const { return m_physicsWorld2D.get(); }
        void SetEnable2DPhysics(bool enable) { m_enable2DPhysics = enable; }
        bool IsEnable2DPhysics() const { return m_enable2DPhysics; }
        
    private:
        std::vector<RigidBody*> m_rigidBodies;
        Vector3 m_gravity = Vector3(0.0f, -9.81f, 0.0f);
        
        // Collision storage (following PhysicsWorld2D pattern)
        std::vector<CollisionInfo> m_collisions;
        int m_collisionCount = 0;
        
        // Spatial partitioning
        std::unique_ptr<Octree> m_octree;
        bool m_useSpatialPartitioning = true;
        
        // Static collider management
        std::vector<ColliderComponent*> m_staticColliders;
        
        // 2D Physics integration
        std::unique_ptr<PhysicsWorld2D> m_physicsWorld2D;
        bool m_enable2DPhysics = true;
        
        // Physics timestep settings
        int m_maxPhysicsStepsPerFrame = 5; // Prevent spiral of death
        
        bool m_initialized = false;
    };
}
