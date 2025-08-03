#pragma once

#include "../ECS/Component.h"
#include "../../Physics/RigidBody/RigidBody.h"
#include <memory>

namespace GameEngine {
    class PhysicsWorld;
    
    class RigidBodyComponent : public Component<RigidBodyComponent> {
    public:
        RigidBodyComponent();
        RigidBodyComponent(PhysicsWorld* physicsWorld);
        ~RigidBodyComponent();
        
        RigidBody* GetRigidBody() const { return m_rigidBody.get(); }
        void SetPhysicsWorld(PhysicsWorld* physicsWorld);
        
        // Collider integration
        void SetColliderComponent(class ColliderComponent* colliderComponent);
        class ColliderComponent* GetColliderComponent() const { return m_colliderComponent; }
        
    private:
        std::unique_ptr<RigidBody> m_rigidBody;
        PhysicsWorld* m_physicsWorld = nullptr;
        class ColliderComponent* m_colliderComponent = nullptr;
    };
}
