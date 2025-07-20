#pragma once

#include "../ECS/System.h"
#include "../ECS/Entity.h"

namespace GameEngine {
    class CameraSystem : public System<CameraSystem> {
    public:
        void OnUpdate(World* world, float deltaTime) override;
        void OnInitialize(World* world) override;
        
    private:
        Entity m_activeCamera;
    };
}
