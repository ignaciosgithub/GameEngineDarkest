#pragma once

#include "../ECS/System.h"

namespace GameEngine {
    class PlayModeManager;
    
    class PhysicsSystem : public System<PhysicsSystem> {
    public:
        PhysicsSystem(PlayModeManager* playModeManager);
        
        void OnInitialize(World* world) override;
        void OnUpdate(World* world, float deltaTime) override;
        
    private:
        void SynchronizePhysicsToTransforms(World* world);
        
        PlayModeManager* m_playModeManager = nullptr;
    };
}
