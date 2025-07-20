#include "CameraSystem.h"
#include "../ECS/World.h"
#include "../Components/CameraComponent.h"
#include "../Components/TransformComponent.h"
#include "../Logging/Logger.h"

namespace GameEngine {

void CameraSystem::OnInitialize(World* /*world*/) {
    Logger::Info("CameraSystem initialized");
}

void CameraSystem::OnUpdate(World* world, float /*deltaTime*/) {
    if (!m_activeCamera.IsValid()) {
        for (const auto& entity : world->GetEntities()) {
            if (world->HasComponent<CameraComponent>(entity)) {
                m_activeCamera = entity;
                Logger::Info("Found active camera entity: " + std::to_string(entity.GetID()));
                break;
            }
        }
    }
    
    if (m_activeCamera.IsValid()) {
        auto* camera = world->GetComponent<CameraComponent>(m_activeCamera);
        auto* transform = world->GetComponent<TransformComponent>(m_activeCamera);
        
        if (camera && transform) {
        }
    }
}

}
