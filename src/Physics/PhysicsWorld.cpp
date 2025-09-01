#include "PhysicsWorld.h"
#include "RigidBody/RigidBody.h"
#include "Collision/CollisionDetection.h"
#include "Spatial/Octree.h"
#include "2D/PhysicsWorld2D.h"
#include "../Core/Logging/Logger.h"
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

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
    
    const int solverIterations = 8;
    for (int it = 0; it < solverIterations; ++it) {
        ResolveCollisions();
    }
    
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

    std::mutex appendMutex;

    auto workerCount = std::max(1u, std::thread::hardware_concurrency());
    if (workerCount <= 1) {
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
        Logger::Debug("Detected " + std::to_string(m_collisionCount) + " total collisions (single-thread)");
        return;
    }

    std::vector<std::thread> threads;

    if (m_useSpatialPartitioning && m_octree) {
        std::vector<std::pair<RigidBody*, RigidBody*>> collisionPairs;
        m_octree->GetCollisionPairs(collisionPairs);
        size_t total = collisionPairs.size();
        size_t chunk = std::max<size_t>(1, (total + workerCount - 1) / workerCount);
        std::atomic<int> localCount{0};
        for (unsigned t = 0; t < workerCount; ++t) {
            size_t start = t * chunk;
            if (start >= total) break;
            size_t end = std::min(total, start + chunk);
            threads.emplace_back([&, start, end]() {
                std::vector<CollisionInfo> local;
                local.reserve(end - start);
                for (size_t i = start; i < end; ++i) {
                    auto& p = collisionPairs[i];
                    if (p.first && p.second) {
                        CollisionInfo info;
                        if (CollisionDetection::CheckCollision(p.first, p.second, info)) {
                            local.push_back(info);
                        }
                    }
                }
                if (!local.empty()) {
                    std::lock_guard<std::mutex> lock(appendMutex);
                    localCount += static_cast<int>(local.size());
                    m_collisions.insert(m_collisions.end(), local.begin(), local.end());
                }
            });
        }
        for (auto& th : threads) th.join();
        threads.clear();
        m_collisionCount += std::max(0, (int)m_collisions.size());
    } else {
        size_t n = m_rigidBodies.size();
        size_t totalPairs = n > 1 ? (n * (n - 1)) / 2 : 0;
        if (totalPairs > 0) {
            size_t chunkPairs = std::max<size_t>(1, (totalPairs + workerCount - 1) / workerCount);
            std::atomic<size_t> pairIndex{0};
            for (unsigned t = 0; t < workerCount; ++t) {
                threads.emplace_back([&, chunkPairs]() {
                    std::vector<CollisionInfo> local;
                    local.reserve(chunkPairs);
                    while (true) {
                        size_t startIdx = pairIndex.fetch_add(chunkPairs);
                        if (startIdx >= totalPairs) break;
                        size_t endIdx = std::min(totalPairs, startIdx + chunkPairs);
                        size_t idx = 0;
                        for (size_t i = 0; i < n; ++i) {
                            for (size_t j = i + 1; j < n; ++j) {
                                if (idx >= endIdx) break;
                                if (idx >= startIdx) {
                                    RigidBody* bodyA = m_rigidBodies[i];
                                    RigidBody* bodyB = m_rigidBodies[j];
                                    if (bodyA && bodyB) {
                                        CollisionInfo info;
                                        if (CollisionDetection::CheckCollision(bodyA, bodyB, info)) {
                                            local.push_back(info);
                                        }
                                    }
                                }
                                ++idx;
                            }
                            if (idx >= endIdx) break;
                        }
                        if (!local.empty()) {
                            std::lock_guard<std::mutex> lock(appendMutex);
                            m_collisions.insert(m_collisions.end(), local.begin(), local.end());
                            m_collisionCount += (int)local.size();
                            local.clear();
                        }
                    }
                });
            }
            for (auto& th : threads) th.join();
            threads.clear();
        }
    }

    {
        size_t nBodies = m_rigidBodies.size();
        size_t nStatics = m_staticColliders.size();
        size_t total = nBodies * nStatics;
        if (total > 0) {
            size_t chunk = std::max<size_t>(1, (total + workerCount - 1) / workerCount);
            std::atomic<size_t> idxAtomic{0};
            for (unsigned t = 0; t < workerCount; ++t) {
                threads.emplace_back([&, chunk, nBodies, nStatics]() {
                    std::vector<CollisionInfo> local;
                    local.reserve(chunk);
                    while (true) {
                        size_t start = idxAtomic.fetch_add(chunk);
                        if (start >= nBodies * nStatics) break;
                        size_t end = std::min(nBodies * nStatics, start + chunk);
                        for (size_t k = start; k < end; ++k) {
                            size_t i = k / nStatics;
                            size_t j = k % nStatics;
                            RigidBody* rigidBody = m_rigidBodies[i];
                            ColliderComponent* collider = m_staticColliders[j];
                            if (rigidBody && collider) {
                                CollisionInfo info;
                                if (CollisionDetection::CheckCollision(rigidBody, collider, info)) {
                                    local.push_back(info);
                                }
                            }
                        }
                        if (!local.empty()) {
                            std::lock_guard<std::mutex> lock(appendMutex);
                            m_collisions.insert(m_collisions.end(), local.begin(), local.end());
                            m_collisionCount += (int)local.size();
                            local.clear();
                        }
                    }
                });
            }
            for (auto& th : threads) th.join();
            threads.clear();
        }
    }

    {
        size_t n = m_staticColliders.size();
        size_t totalPairs = n > 1 ? (n * (n - 1)) / 2 : 0;
        if (totalPairs > 0) {
            size_t chunkPairs = std::max<size_t>(1, (totalPairs + workerCount - 1) / workerCount);
            std::atomic<size_t> pairIndex{0};
            for (unsigned t = 0; t < workerCount; ++t) {
                threads.emplace_back([&, chunkPairs, n]() {
                    std::vector<CollisionInfo> local;
                    local.reserve(chunkPairs);
                    while (true) {
                        size_t startIdx = pairIndex.fetch_add(chunkPairs);
                        if (startIdx >= (n * (n - 1)) / 2) break;
                        size_t endIdx = std::min((n * (n - 1)) / 2, startIdx + chunkPairs);
                        size_t idx = 0;
                        for (size_t i = 0; i < n; ++i) {
                            for (size_t j = i + 1; j < n; ++j) {
                                if (idx >= endIdx) break;
                                if (idx >= startIdx) {
                                    ColliderComponent* colliderA = m_staticColliders[i];
                                    ColliderComponent* colliderB = m_staticColliders[j];
                                    if (colliderA && colliderB) {
                                        CollisionInfo info;
                                        if (CollisionDetection::CheckCollision(colliderA, colliderB, info)) {
                                            local.push_back(info);
                                        }
                                    }
                                }
                                ++idx;
                            }
                            if (idx >= endIdx) break;
                        }
                        if (!local.empty()) {
                            std::lock_guard<std::mutex> lock(appendMutex);
                            m_collisions.insert(m_collisions.end(), local.begin(), local.end());
                            m_collisionCount += (int)local.size();
                            local.clear();
                        }
                    }
                });
            }
            for (auto& th : threads) th.join();
            threads.clear();
        }
    }

    Logger::Debug("Detected " + std::to_string(m_collisionCount) + " total collisions (multi-thread)");
}

void PhysicsWorld::ResolveCollisions() {
    if (m_collisions.empty()) return;
    auto workerCount = std::max(1u, std::thread::hardware_concurrency());
    if (workerCount <= 1) {
        for (const auto& collision : m_collisions) {
            if (!collision.hasCollision) continue;
            CollisionDetection::ResolveCollision(collision.bodyA, collision.bodyB, collision);
        }
        return;
    }
    size_t total = m_collisions.size();
    size_t chunk = std::max<size_t>(1, (total + workerCount - 1) / workerCount);
    std::vector<std::thread> threads;
    for (unsigned t = 0; t < workerCount; ++t) {
        size_t start = t * chunk;
        if (start >= total) break;
        size_t end = std::min(total, start + chunk);
        threads.emplace_back([this, start, end]() {
            for (size_t i = start; i < end; ++i) {
                const auto& collision = m_collisions[i];
                if (!collision.hasCollision) continue;
                CollisionDetection::ResolveCollision(collision.bodyA, collision.bodyB, collision);
            }
        });
    }
    for (auto& th : threads) th.join();
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
    auto workerCount = std::max(1u, std::thread::hardware_concurrency());
    if (workerCount <= 1) {
        for (RigidBody* body : m_rigidBodies) {
            if (body && !body->IsStatic()) {
                body->AddForce(m_gravity * body->GetMass());
                body->IntegrateVelocity(deltaTime);
                body->ClearForces();
            }
        }
        return;
    }
    size_t total = m_rigidBodies.size();
    size_t chunk = std::max<size_t>(1, (total + workerCount - 1) / workerCount);
    std::vector<std::thread> threads;
    for (unsigned t = 0; t < workerCount; ++t) {
        size_t start = t * chunk;
        if (start >= total) break;
        size_t end = std::min(total, start + chunk);
        threads.emplace_back([this, start, end, deltaTime]() {
            for (size_t i = start; i < end; ++i) {
                RigidBody* body = m_rigidBodies[i];
                if (body && !body->IsStatic()) {
                    body->AddForce(m_gravity * body->GetMass());
                    body->IntegrateVelocity(deltaTime);
                    body->ClearForces();
                }
            }
        });
    }
    for (auto& th : threads) th.join();
}

void PhysicsWorld::IntegratePositions(float deltaTime) {
    auto workerCount = std::max(1u, std::thread::hardware_concurrency());
    if (workerCount <= 1) {
        for (RigidBody* body : m_rigidBodies) {
            if (body && !body->IsStatic()) {
                Vector3 oldPosition = body->GetPosition();
                body->IntegratePosition(deltaTime);
                if (m_octree && m_useSpatialPartitioning) {
                    Vector3 displacement = body->GetPosition() - oldPosition;
                    if (displacement.Length() > 0.1f) {
                        m_octree->Update(body);
                    }
                }
            }
        }
        return;
    }
    size_t total = m_rigidBodies.size();
    size_t chunk = std::max<size_t>(1, (total + workerCount - 1) / workerCount);
    std::vector<std::thread> threads;
    for (unsigned t = 0; t < workerCount; ++t) {
        size_t start = t * chunk;
        if (start >= total) break;
        size_t end = std::min(total, start + chunk);
        threads.emplace_back([this, start, end, deltaTime]() {
            for (size_t i = start; i < end; ++i) {
                RigidBody* body = m_rigidBodies[i];
                if (body && !body->IsStatic()) {
                    Vector3 oldPosition = body->GetPosition();
                    body->IntegratePosition(deltaTime);
                    if (m_octree && m_useSpatialPartitioning) {
                        Vector3 displacement = body->GetPosition() - oldPosition;
                        if (displacement.Length() > 0.1f) {
                            m_octree->Update(body);
                        }
                    }
                }
            }
        });
    }
    for (auto& th : threads) th.join();
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
