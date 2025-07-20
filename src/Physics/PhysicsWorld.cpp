#include "PhysicsWorld.h"
#include "RigidBody/RigidBody.h"
#include "Collision/CollisionDetection.h"
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
    m_initialized = true;
    
    Logger::Info("PhysicsWorld initialized");
}

void PhysicsWorld::Shutdown() {
    if (m_initialized) {
        m_rigidBodies.clear();
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
}

void PhysicsWorld::AddRigidBody(RigidBody* rigidBody) {
    if (!rigidBody) return;
    
    auto it = std::find(m_rigidBodies.begin(), m_rigidBodies.end(), rigidBody);
    if (it == m_rigidBodies.end()) {
        m_rigidBodies.push_back(rigidBody);
        Logger::Debug("Added RigidBody to PhysicsWorld");
    }
}

void PhysicsWorld::RemoveRigidBody(RigidBody* rigidBody) {
    if (!rigidBody) return;
    
    auto it = std::find(m_rigidBodies.begin(), m_rigidBodies.end(), rigidBody);
    if (it != m_rigidBodies.end()) {
        m_rigidBodies.erase(it);
        Logger::Debug("Removed RigidBody from PhysicsWorld");
    }
}

void PhysicsWorld::DetectCollisions() {
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
            Vector3 position = body->GetPosition() + body->GetVelocity() * deltaTime;
            body->SetPosition(position);
        }
    }
}

}
