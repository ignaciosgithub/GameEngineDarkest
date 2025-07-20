#pragma once

#include <vector>
#include <memory>
#include "../Core/Math/Vector3.h"

namespace GameEngine {
    class RigidBody;
    class Entity;
    class World;
    
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
        
        // Collision detection
        void DetectCollisions();
        void ResolveCollisions();
        
        // Integration
        void IntegrateVelocities(float deltaTime);
        void IntegratePositions(float deltaTime);
        
    private:
        std::vector<RigidBody*> m_rigidBodies;
        Vector3 m_gravity = Vector3(0.0f, -9.81f, 0.0f);
        
        bool m_initialized = false;
    };
}
