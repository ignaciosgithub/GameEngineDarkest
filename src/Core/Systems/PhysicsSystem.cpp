#include "PhysicsSystem.h"
#include "../ECS/World.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
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

}
