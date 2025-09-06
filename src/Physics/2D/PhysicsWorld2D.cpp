#include "PhysicsWorld2D.h"
#include "RigidBody2D.h"
#include "Collision/CollisionDetection2D.h"
#include "../../Core/Logging/Logger.h"
#include "../../Core/Profiling/Profiler.h"
#include <algorithm>

namespace GameEngine {

PhysicsWorld2D::PhysicsWorld2D() {
}

PhysicsWorld2D::~PhysicsWorld2D() {
    Shutdown();
}

void PhysicsWorld2D::Initialize() {
    if (m_initialized) return;
    
    Logger::Info("Initializing 2D Physics World...");
    
    Vector2 worldCenter = (m_worldMin + m_worldMax) * 0.5f;
    Vector2 worldHalfSize = (m_worldMax - m_worldMin) * 0.5f;
    QuadTreeBounds worldBounds(worldCenter, worldHalfSize);
    m_quadTree = std::make_unique<QuadTree>(0, worldBounds);
    
    m_initialized = true;
    Logger::Info("2D Physics World initialized successfully");
}

void PhysicsWorld2D::Shutdown() {
    if (!m_initialized) return;
    
    Logger::Info("Shutting down 2D Physics World...");
    
    m_rigidBodies.clear();
    m_collisions.clear();
    m_quadTree.reset();
    
    m_initialized = false;
    Logger::Info("2D Physics World shutdown complete");
}

void PhysicsWorld2D::Update(float deltaTime) {
    if (!m_initialized) return;
    
    ApplyGravity(deltaTime);
    
    if (m_useSpatialPartitioning) {
        UpdateSpatialPartitioning();
    }
    
    DetectCollisions();
    
    for (int i = 0; i < m_velocityIterations; ++i) {
        PROFILE_SCOPE("Physics2D::ResolveCollisions");
        ResolveCollisions();
    }
    
    {
        PROFILE_SCOPE("Physics2D::IntegrateVelocities");
        IntegrateVelocities(deltaTime);
    }
    
    for (int i = 0; i < m_positionIterations; ++i) {
        DetectCollisions();
        {
            PROFILE_SCOPE("Physics2D::ResolveCollisions");
            ResolveCollisions();
        }
    }
    
    {
        PROFILE_SCOPE("Physics2D::IntegratePositions");
        IntegratePositions(deltaTime);
    }
}

void PhysicsWorld2D::FixedUpdate(float fixedDeltaTime) {
    Update(fixedDeltaTime);
}

void PhysicsWorld2D::AddRigidBody(RigidBody2D* rigidBody) {
    if (!rigidBody) return;
    
    auto it = std::find(m_rigidBodies.begin(), m_rigidBodies.end(), rigidBody);
    if (it == m_rigidBodies.end()) {
        m_rigidBodies.push_back(rigidBody);
        Logger::Debug("Added RigidBody2D to physics world");
    }
}

void PhysicsWorld2D::RemoveRigidBody(RigidBody2D* rigidBody) {
    if (!rigidBody) return;
    
    auto it = std::find(m_rigidBodies.begin(), m_rigidBodies.end(), rigidBody);
    if (it != m_rigidBodies.end()) {
        m_rigidBodies.erase(it);
        Logger::Debug("Removed RigidBody2D from physics world");
    }
}

void PhysicsWorld2D::DetectCollisions() {
    m_collisions.clear();
    m_collisionCount = 0;
    
    if (m_useSpatialPartitioning && m_quadTree) {
        BroadPhaseCollisionDetection();
    } else {
        for (size_t i = 0; i < m_rigidBodies.size(); ++i) {
            for (size_t j = i + 1; j < m_rigidBodies.size(); ++j) {
                CollisionInfo2D info;
                if (CollisionDetection2D::CheckCollision(m_rigidBodies[i], m_rigidBodies[j], info)) {
                    m_collisions.push_back(info);
                    m_collisionCount++;
                }
            }
        }
    }
}

void PhysicsWorld2D::ResolveCollisions() {
    PROFILE_SCOPE("Physics2D::ResolveCollisions");
    for (const auto& collision : m_collisions) {
        CollisionDetection2D::ResolveCollision(collision.bodyA, collision.bodyB, collision);
    }
}

void PhysicsWorld2D::IntegrateVelocities(float deltaTime) {
    PROFILE_SCOPE("Physics2D::IntegrateVelocities");
    for (RigidBody2D* body : m_rigidBodies) {
        if (body && body->IsDynamic()) {
            body->IntegrateVelocity(deltaTime);
        }
    }
}

void PhysicsWorld2D::IntegratePositions(float deltaTime) {
    PROFILE_SCOPE("Physics2D::IntegratePositions");
    for (RigidBody2D* body : m_rigidBodies) {
        if (body && body->IsDynamic()) {
            body->IntegratePosition(deltaTime);
        }
    }
}

void PhysicsWorld2D::UpdateSpatialPartitioning() {
    if (!m_quadTree) return;
    
    m_quadTree->Clear();
    
    for (RigidBody2D* body : m_rigidBodies) {
        if (body) {
            m_quadTree->Insert(body);
        }
    }
}

void PhysicsWorld2D::SetWorldBounds(const Vector2& min, const Vector2& max) {
    m_worldMin = min;
    m_worldMax = max;
    
    if (m_quadTree) {
        Vector2 worldCenter = (m_worldMin + m_worldMax) * 0.5f;
        Vector2 worldHalfSize = (m_worldMax - m_worldMin) * 0.5f;
        QuadTreeBounds worldBounds(worldCenter, worldHalfSize);
        m_quadTree = std::make_unique<QuadTree>(0, worldBounds);
    }
}

int PhysicsWorld2D::GetActiveBodyCount() const {
    int count = 0;
    for (const RigidBody2D* body : m_rigidBodies) {
        if (body && !body->IsSleeping()) {
            count++;
        }
    }
    return count;
}

bool PhysicsWorld2D::Raycast(const Vector2& start, const Vector2& end, RigidBody2D*& hitBody, Vector2& hitPoint, Vector2& hitNormal) {
    hitBody = nullptr;
    float closestDistance = (end - start).Length();
    bool hit = false;
    
    for (RigidBody2D* body : m_rigidBodies) {
        if (!body) continue;
        
        if (body->GetColliderType() == Collider2DType::Circle) {
            Vector2 center = body->GetPosition();
            float radius = body->GetColliderRadius();
            
            Vector2 rayDir = (end - start).Normalized();
            Vector2 toCenter = center - start;
            float projLength = toCenter.Dot(rayDir);
            
            if (projLength < 0 || projLength > closestDistance) continue;
            
            Vector2 closestPoint = start + rayDir * projLength;
            float distanceToCenter = (center - closestPoint).Length();
            
            if (distanceToCenter <= radius) {
                float hitDistance = projLength - std::sqrt(radius * radius - distanceToCenter * distanceToCenter);
                if (hitDistance >= 0 && hitDistance < closestDistance) {
                    closestDistance = hitDistance;
                    hitBody = body;
                    hitPoint = start + rayDir * hitDistance;
                    hitNormal = (hitPoint - center).Normalized();
                    hit = true;
                }
            }
        }
    }
    
    return hit;
}

void PhysicsWorld2D::ApplyGravity(float /*deltaTime*/) {
    for (RigidBody2D* body : m_rigidBodies) {
        if (body && body->IsDynamic()) {
            Vector2 gravityForce = m_gravity * body->GetMass();
            body->AddForce(gravityForce);
        }
    }
}

void PhysicsWorld2D::BroadPhaseCollisionDetection() {
    if (!m_quadTree) return;
    
    for (size_t i = 0; i < m_rigidBodies.size(); ++i) {
        RigidBody2D* bodyA = m_rigidBodies[i];
        if (!bodyA) continue;
        
        std::vector<RigidBody2D*> potentialCollisions;
        m_quadTree->Retrieve(potentialCollisions, bodyA);
        
        for (RigidBody2D* bodyB : potentialCollisions) {
            if (bodyA != bodyB) {
                CollisionInfo2D info;
                if (CollisionDetection2D::CheckCollision(bodyA, bodyB, info)) {
                    bool exists = false;
                    for (const auto& existing : m_collisions) {
                        if ((existing.bodyA == bodyA && existing.bodyB == bodyB) ||
                            (existing.bodyA == bodyB && existing.bodyB == bodyA)) {
                            exists = true;
                            break;
                        }
                    }
                    
                    if (!exists) {
                        m_collisions.push_back(info);
                        m_collisionCount++;
                    }
                }
            }
        }
    }
}

}
