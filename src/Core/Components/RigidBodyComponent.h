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
        
    private:
        std::unique_ptr<RigidBody> m_rigidBody;
        PhysicsWorld* m_physicsWorld = nullptr;
    };
}
