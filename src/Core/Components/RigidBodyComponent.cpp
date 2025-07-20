#include "RigidBodyComponent.h"

namespace GameEngine {

RigidBodyComponent::RigidBodyComponent() 
    : m_rigidBody(std::make_unique<RigidBody>()) {
}

RigidBodyComponent::~RigidBodyComponent() = default;

}
