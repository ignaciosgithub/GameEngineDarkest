#pragma once

#include "../ECS/Component.h"
#include "../../Physics/RigidBody/RigidBody.h"
#include <memory>

namespace GameEngine {
    class RigidBodyComponent : public Component<RigidBodyComponent> {
    public:
        RigidBodyComponent();
        ~RigidBodyComponent();
        
        RigidBody* GetRigidBody() const { return m_rigidBody.get(); }
        
    private:
        std::unique_ptr<RigidBody> m_rigidBody;
    };
}
