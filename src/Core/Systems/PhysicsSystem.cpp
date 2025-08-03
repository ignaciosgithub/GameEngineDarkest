#include "PhysicsSystem.h"
#include "../ECS/World.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/ColliderComponent.h"
#include "../Editor/PlayModeManager.h"
#include "../Logging/Logger.h"

namespace GameEngine {

PhysicsSystem::PhysicsSystem(PlayModeManager* playModeManager)
    : m_playModeManager(playModeManager) {
}

void PhysicsSystem::OnInitialize(World* /*world*/) {
    Logger::Info("PhysicsSystem initialized");
}

void PhysicsSystem::OnUpdate(World* world, float /*deltaTime*/) {
    if (m_playModeManager && m_playModeManager->IsInPlayMode()) {
        SynchronizePhysicsToTransforms(world);
        UpdateColliderPhysicsIntegration(world);
    }
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
                if (rigidBody->IsStatic()) {
                    Vector3 transformPosition = transformComp->transform.GetPosition();
                    rigidBody->SetPosition(transformPosition);
                }
                
                if (!rigidBodyComp->GetColliderComponent()) {
                    rigidBodyComp->SetColliderComponent(colliderComp);
                    Logger::Debug("Linked ColliderComponent to RigidBody for entity: " + std::to_string(entity.GetID()));
                }
            }
        }
        
        else if (colliderComp && transformComp && !rigidBodyComp) {
            if (colliderComp->HasCollider()) {
                Logger::Debug("Static collider detected for entity: " + std::to_string(entity.GetID()));
            }
        }
    }
}

}
