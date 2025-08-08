#include "PhysicsSystem.h"
#include "../ECS/World.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/ColliderComponent.h"
#include "../Editor/PlayModeManager.h"
#include "../Logging/Logger.h"

namespace GameEngine {

PhysicsSystem::PhysicsSystem(PlayModeManager* playModeManager, PhysicsWorld* physicsWorld)
    : m_playModeManager(playModeManager), m_physicsWorld(physicsWorld) {
}

void PhysicsSystem::OnInitialize(World* /*world*/) {
    Logger::Info("PhysicsSystem initialized");
}

void PhysicsSystem::OnUpdate(World* world, float /*deltaTime*/) {
    if (m_playModeManager && m_playModeManager->IsInPlayMode()) {
        SynchronizePhysicsToTransforms(world);
    }
    UpdateColliderPhysicsIntegration(world);
    CleanupStaticColliders(world);
}

void PhysicsSystem::SynchronizePhysicsToTransforms(World* world) {
    for (const auto& entity : world->GetEntities()) {
        auto* rigidBodyComp = world->GetComponent<RigidBodyComponent>(entity);
        auto* transformComp = world->GetComponent<TransformComponent>(entity);
        
        if (rigidBodyComp && transformComp) {
            RigidBody* rigidBody = rigidBodyComp->GetRigidBody();
            if (rigidBody && !rigidBody->IsStatic()) {
                Vector3 physicsPosition = rigidBody->GetPosition();
                transformComp->transform.SetPosition(physicsPosition);
                transformComp->transform.SetRotation(rigidBody->GetRotation());
            }
        }
    }
}

void PhysicsSystem::UpdateColliderPhysicsIntegration(World* world) {
    for (const auto& entity : world->GetEntities()) {
        auto* rigidBodyComp = world->GetComponent<RigidBodyComponent>(entity);
        auto* colliderComp = world->GetComponent<ColliderComponent>(entity);
        auto* transformComp = world->GetComponent<TransformComponent>(entity);
        
        if (rigidBodyComp && colliderComp && transformComp) {
            RigidBody* rigidBody = rigidBodyComp->GetRigidBody();
            if (rigidBody && colliderComp->HasCollider()) {
                colliderComp->SetOwnerTransform(transformComp);
                if (rigidBody->IsStatic()) {
                    Vector3 transformPosition = transformComp->transform.GetPosition();
                    rigidBody->SetPosition(transformPosition);
                    rigidBody->SetRotation(transformComp->transform.GetRotation());
                }
                
                if (!rigidBodyComp->GetColliderComponent()) {
                    rigidBodyComp->SetColliderComponent(colliderComp);
                    Logger::Debug("Linked ColliderComponent to RigidBody for entity: " + std::to_string(entity.GetID()));
                }
                if (!rigidBody->GetTransformComponent()) {
                    rigidBody->SetTransformComponent(transformComp);
                    Logger::Debug("Linked TransformComponent to RigidBody for entity: " + std::to_string(entity.GetID()));
                }
            }
        }
        
        else if (colliderComp && transformComp && !rigidBodyComp) {
            colliderComp->SetOwnerTransform(transformComp);
            if (colliderComp->HasCollider()) {
                if (m_physicsWorld && m_registeredStaticColliders.find(colliderComp) == m_registeredStaticColliders.end()) {
                    m_physicsWorld->AddStaticCollider(colliderComp);
                    m_registeredStaticColliders.insert(colliderComp);
                    Logger::Debug("Registered static collider with PhysicsWorld for entity: " + std::to_string(entity.GetID()));
                } else if (!m_physicsWorld) {
                    Logger::Warning("PhysicsWorld not available - cannot register static collider for entity: " + std::to_string(entity.GetID()));
                }
            }
        }
    }
}

void PhysicsSystem::CleanupStaticColliders(World* world) {
    if (!m_physicsWorld) return;
    
    auto it = m_registeredStaticColliders.begin();
    while (it != m_registeredStaticColliders.end()) {
        ColliderComponent* collider = *it;
        
        bool shouldRemove = false;
        
        bool entityExists = false;
        bool hasRigidBody = false;
        
        for (const auto& entity : world->GetEntities()) {
            auto* entityCollider = world->GetComponent<ColliderComponent>(entity);
            if (entityCollider == collider) {
                entityExists = true;
                auto* rigidBodyComp = world->GetComponent<RigidBodyComponent>(entity);
                if (rigidBodyComp) {
                    hasRigidBody = true;
                }
                break;
            }
        }
        
        if (!entityExists || hasRigidBody || !collider->HasCollider()) {
            shouldRemove = true;
        }
        
        if (shouldRemove) {
            m_physicsWorld->RemoveStaticCollider(collider);
            it = m_registeredStaticColliders.erase(it);
            Logger::Debug("Removed static collider from PhysicsWorld during cleanup");
        } else {
            ++it;
        }
    }
}

}
