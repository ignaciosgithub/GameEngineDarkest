#pragma once

#include "UIPanel.h"
#include "../../Core/ECS/Entity.h"

namespace GameEngine {
    class InspectorPanel : public UIPanel {
    public:
        InspectorPanel();
        ~InspectorPanel() override;
        
        void Update(World* world, float deltaTime) override;
        
        void SetSelectedEntity(Entity entity) { m_selectedEntity = entity; }
        
    private:
        void DrawTransformComponent(World* world, Entity entity);
        void DrawCameraComponent(World* world, Entity entity);
        void DrawMovementComponent(World* world, Entity entity);
        void DrawMeshComponent(World* world, Entity entity);
        void DrawRigidBodyComponent(World* world, Entity entity);
        void DrawAudioComponent(World* world, Entity entity);
        void DrawLightComponent(World* world, Entity entity);
        
        // Component addition methods
        void AddCameraComponent(World* world, Entity entity);
        void AddMovementComponent(World* world, Entity entity);
        void AddMeshComponent(World* world, Entity entity);
        void AddRigidBodyComponent(World* world, Entity entity);
        void AddAudioComponent(World* world, Entity entity);
        void AddLightComponent(World* world, Entity entity);
        
        // Component removal methods
        void RemoveCameraComponent(World* world, Entity entity);
        void RemoveMovementComponent(World* world, Entity entity);
        void RemoveMeshComponent(World* world, Entity entity);
        void RemoveRigidBodyComponent(World* world, Entity entity);
        void RemoveAudioComponent(World* world, Entity entity);
        void RemoveLightComponent(World* world, Entity entity);
        
        Entity m_selectedEntity;
    };
}
