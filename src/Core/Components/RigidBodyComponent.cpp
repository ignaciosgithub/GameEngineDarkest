#include "RigidBodyComponent.h"
#include "ColliderComponent.h"
#include "../../Physics/PhysicsWorld.h"

namespace GameEngine {

RigidBodyComponent::RigidBodyComponent() 
    : m_rigidBody(std::make_unique<RigidBody>()) {
}

RigidBodyComponent::RigidBodyComponent(PhysicsWorld* physicsWorld) 
    : m_rigidBody(std::make_unique<RigidBody>()), m_physicsWorld(physicsWorld) {
    if (m_physicsWorld && m_rigidBody) {
        m_physicsWorld->AddRigidBody(m_rigidBody.get());
    }
}

RigidBodyComponent::~RigidBodyComponent() {
    if (m_physicsWorld && m_rigidBody) {
        m_physicsWorld->RemoveRigidBody(m_rigidBody.get());
    }
}

void RigidBodyComponent::SetPhysicsWorld(PhysicsWorld* physicsWorld) {
    if (m_physicsWorld && m_rigidBody) {
        m_physicsWorld->RemoveRigidBody(m_rigidBody.get());
    }
    
    m_physicsWorld = physicsWorld;
    
    if (m_physicsWorld && m_rigidBody) {
        m_physicsWorld->AddRigidBody(m_rigidBody.get());
    }
}

void RigidBodyComponent::SetColliderComponent(ColliderComponent* colliderComponent) {
    m_colliderComponent = colliderComponent;
    
    if (m_rigidBody) {
        m_rigidBody->SetColliderComponent(colliderComponent);
    }
}

}
