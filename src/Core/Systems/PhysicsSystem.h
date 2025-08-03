#pragma once

#include "../ECS/System.h"
#include "../Components/ColliderComponent.h"
#include "../../Physics/PhysicsWorld.h"
#include <set>

namespace GameEngine {
    class PlayModeManager;
    
    class PhysicsSystem : public System<PhysicsSystem> {
    public:
        PhysicsSystem(PlayModeManager* playModeManager, PhysicsWorld* physicsWorld);
        
        void OnInitialize(World* world) override;
        void OnUpdate(World* world, float deltaTime) override;
        
    private:
        void SynchronizePhysicsToTransforms(World* world);
        void UpdateColliderPhysicsIntegration(World* world);
        void CleanupStaticColliders(World* world);
        
        PlayModeManager* m_playModeManager = nullptr;
        PhysicsWorld* m_physicsWorld = nullptr;
        std::set<ColliderComponent*> m_registeredStaticColliders;
    };
}
