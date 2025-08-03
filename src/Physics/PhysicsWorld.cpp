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
    
    int stepCount = 0;
    while (accumulator >= fixedDeltaTime && stepCount < m_maxPhysicsStepsPerFrame) {
        FixedUpdate(fixedDeltaTime);
        accumulator -= fixedDeltaTime;
        stepCount++;
    }
    
    if (stepCount >= m_maxPhysicsStepsPerFrame) {
        Logger::Warning("Physics accumulator hit max steps limit (" + std::to_string(m_maxPhysicsStepsPerFrame) + ") with deltaTime: " + std::to_string(deltaTime));
        accumulator = 0.0f;
    }
    
    if (stepCount > 1) {
        Logger::Debug("Physics processed " + std::to_string(stepCount) + " steps in single frame");
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
    m_collisions.clear();
    m_collisionCount = 0;
    
    if (m_useSpatialPartitioning && m_octree) {
        std::vector<std::pair<RigidBody*, RigidBody*>> collisionPairs;
        m_octree->GetCollisionPairs(collisionPairs);
        
        for (const auto& pair : collisionPairs) {
            if (pair.first && pair.second) {
                CollisionInfo info;
                if (CollisionDetection::CheckCollision(pair.first, pair.second, info)) {
                    m_collisions.push_back(info);
                    m_collisionCount++;
                }
            }
        }
        
        Logger::Debug("Spatial partitioning detected " + std::to_string(m_collisionCount) + " RigidBody vs RigidBody collisions");
    } else {
        for (size_t i = 0; i < m_rigidBodies.size(); ++i) {
            for (size_t j = i + 1; j < m_rigidBodies.size(); ++j) {
                RigidBody* bodyA = m_rigidBodies[i];
                RigidBody* bodyB = m_rigidBodies[j];
                
                if (bodyA && bodyB) {
                    CollisionInfo info;
                    if (CollisionDetection::CheckCollision(bodyA, bodyB, info)) {
                        m_collisions.push_back(info);
                        m_collisionCount++;
                    }
                }
            }
        }
    }
    
    for (RigidBody* rigidBody : m_rigidBodies) {
        for (ColliderComponent* collider : m_staticColliders) {
            if (rigidBody && collider) {
                CollisionInfo info;
                if (CollisionDetection::CheckCollision(rigidBody, collider, info)) {
                    m_collisions.push_back(info);
                    m_collisionCount++;
                }
            }
        }
    }
    
    for (size_t i = 0; i < m_staticColliders.size(); ++i) {
        for (size_t j = i + 1; j < m_staticColliders.size(); ++j) {
            ColliderComponent* colliderA = m_staticColliders[i];
            ColliderComponent* colliderB = m_staticColliders[j];
            
            if (colliderA && colliderB) {
                CollisionInfo info;
                if (CollisionDetection::CheckCollision(colliderA, colliderB, info)) {
                    m_collisions.push_back(info);
                    m_collisionCount++;
                }
            }
        }
    }
    
    Logger::Debug("Detected " + std::to_string(m_collisionCount) + " total collisions (RigidBody vs RigidBody, RigidBody vs ColliderComponent, ColliderComponent vs ColliderComponent)");
}

void PhysicsWorld::ResolveCollisions() {
    for (const auto& collision : m_collisions) {
        if (collision.hasCollision && collision.bodyA && collision.bodyB) {
            CollisionDetection::ResolveCollision(collision.bodyA, collision.bodyB, collision);
        }
    }
}

void PhysicsWorld::AddStaticCollider(ColliderComponent* collider) {
    if (!collider) return;
    
    auto it = std::find(m_staticColliders.begin(), m_staticColliders.end(), collider);
    if (it == m_staticColliders.end()) {
        m_staticColliders.push_back(collider);
        
        if (m_octree && m_useSpatialPartitioning) {
        }
        
        Logger::Debug("Added static ColliderComponent to PhysicsWorld");
    }
}

void PhysicsWorld::RemoveStaticCollider(ColliderComponent* collider) {
    if (!collider) return;
    
    auto it = std::find(m_staticColliders.begin(), m_staticColliders.end(), collider);
    if (it != m_staticColliders.end()) {
        m_staticColliders.erase(it);
        
        if (m_octree && m_useSpatialPartitioning) {
        }
        
        Logger::Debug("Removed static ColliderComponent from PhysicsWorld");
    }
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
