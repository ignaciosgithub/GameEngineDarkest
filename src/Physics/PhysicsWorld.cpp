#include "PhysicsWorld.h"
#include "RigidBody/RigidBody.h"
#include "Collision/CollisionDetection.h"
#include "Spatial/Octree.h"
#include "2D/PhysicsWorld2D.h"
#include "../Core/Logging/Logger.h"
#include <algorithm>

namespace GameEngine {

PhysicsWorld::PhysicsWorld() = default;

PhysicsWorld::~PhysicsWorld() {
    Shutdown();
}

void PhysicsWorld::Initialize() {
    if (m_initialized) {
        Logger::Warning("PhysicsWorld already initialized");
        return;
    }
    
    m_rigidBodies.clear();
    
    AABB worldBounds(Vector3(-1000, -1000, -1000), Vector3(1000, 1000, 1000));
    m_octree = std::make_unique<Octree>(worldBounds);
    
    if (m_enable2DPhysics) {
        m_physicsWorld2D = std::make_unique<PhysicsWorld2D>();
        m_physicsWorld2D->Initialize();
        Logger::Info("2D Physics World initialized");
    }
    
    m_initialized = true;
    
    Logger::Info("PhysicsWorld initialized with spatial partitioning");
}

void PhysicsWorld::Shutdown() {
    if (m_initialized) {
        m_rigidBodies.clear();
        m_octree.reset();
        
        if (m_physicsWorld2D) {
            m_physicsWorld2D->Shutdown();
            m_physicsWorld2D.reset();
            Logger::Info("2D Physics World shutdown");
        }
        
        m_initialized = false;
        
        Logger::Info("PhysicsWorld shutdown");
    }
}

void PhysicsWorld::Update(float deltaTime) {
    if (!m_initialized) return;
    
    const float fixedDeltaTime = 1.0f / 60.0f; // 60 FPS physics
    static float accumulator = 0.0f;
    
    accumulator += deltaTime;
    
    while (accumulator >= fixedDeltaTime) {
        FixedUpdate(fixedDeltaTime);
        accumulator -= fixedDeltaTime;
    }
}

void PhysicsWorld::FixedUpdate(float fixedDeltaTime) {
    IntegrateVelocities(fixedDeltaTime);
    
    DetectCollisions();
    ResolveCollisions();
    
    IntegratePositions(fixedDeltaTime);
    
    if (m_enable2DPhysics && m_physicsWorld2D) {
        m_physicsWorld2D->FixedUpdate(fixedDeltaTime);
    }
}

void PhysicsWorld::AddRigidBody(RigidBody* rigidBody) {
    if (!rigidBody) return;
    
    auto it = std::find(m_rigidBodies.begin(), m_rigidBodies.end(), rigidBody);
    if (it == m_rigidBodies.end()) {
        m_rigidBodies.push_back(rigidBody);
        
        if (m_octree && m_useSpatialPartitioning) {
            m_octree->Insert(rigidBody);
        }
        
        Logger::Debug("Added RigidBody to PhysicsWorld");
    }
}

void PhysicsWorld::RemoveRigidBody(RigidBody* rigidBody) {
    if (!rigidBody) return;
    
    auto it = std::find(m_rigidBodies.begin(), m_rigidBodies.end(), rigidBody);
    if (it != m_rigidBodies.end()) {
        m_rigidBodies.erase(it);
        
        if (m_octree && m_useSpatialPartitioning) {
            m_octree->Remove(rigidBody);
        }
        
        Logger::Debug("Removed RigidBody from PhysicsWorld");
    }
}

void PhysicsWorld::DetectCollisions() {
    if (m_useSpatialPartitioning && m_octree) {
        std::vector<std::pair<RigidBody*, RigidBody*>> collisionPairs;
        m_octree->GetCollisionPairs(collisionPairs);
        
        for (const auto& pair : collisionPairs) {
            if (pair.first && pair.second) {
                CollisionDetection::CheckCollision(pair.first, pair.second);
            }
        }
        
        Logger::Debug("Spatial partitioning detected " + std::to_string(collisionPairs.size()) + " potential collision pairs");
    } else {
        for (size_t i = 0; i < m_rigidBodies.size(); ++i) {
            for (size_t j = i + 1; j < m_rigidBodies.size(); ++j) {
                RigidBody* bodyA = m_rigidBodies[i];
                RigidBody* bodyB = m_rigidBodies[j];
                
                if (bodyA && bodyB) {
                    CollisionDetection::CheckCollision(bodyA, bodyB);
                }
            }
        }
    }
}

void PhysicsWorld::ResolveCollisions() {
}

void PhysicsWorld::IntegrateVelocities(float deltaTime) {
    for (RigidBody* body : m_rigidBodies) {
        if (body && !body->IsStatic()) {
            Vector3 acceleration = m_gravity;
            
            if (body->GetMass() > 0.0f) {
                acceleration += body->GetForce() / body->GetMass();
            }
            
            Vector3 velocity = body->GetVelocity() + acceleration * deltaTime;
            body->SetVelocity(velocity);
            
            velocity *= (1.0f - body->GetDamping() * deltaTime);
            body->SetVelocity(velocity);
            
            body->ClearForces();
        }
    }
}

void PhysicsWorld::IntegratePositions(float deltaTime) {
    for (RigidBody* body : m_rigidBodies) {
        if (body && !body->IsStatic()) {
            Vector3 oldPosition = body->GetPosition();
            Vector3 position = oldPosition + body->GetVelocity() * deltaTime;
            body->SetPosition(position);
            
            if (m_octree && m_useSpatialPartitioning) {
                Vector3 displacement = position - oldPosition;
                if (displacement.Length() > 0.1f) { // Threshold for spatial update
                    m_octree->Update(body);
                }
            }
        }
    }
}

void PhysicsWorld::UpdateSpatialPartitioning() {
    if (m_octree && m_useSpatialPartitioning) {
        m_octree->Clear();
        for (RigidBody* body : m_rigidBodies) {
            if (body) {
                m_octree->Insert(body);
            }
        }
    }
}

}
