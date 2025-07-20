#include "PhysicsMaterial.h"
#include <algorithm>
#include <memory>

namespace GameEngine {

PhysicsMaterial::PhysicsMaterial(const std::string& name) : m_name(name) {
}

float PhysicsMaterial::CombineFriction(float friction1, float friction2, FrictionCombine mode) {
    switch (mode) {
        case FrictionCombine::Average:
            return (friction1 + friction2) * 0.5f;
        case FrictionCombine::Minimum:
            return std::min(friction1, friction2);
        case FrictionCombine::Maximum:
            return std::max(friction1, friction2);
        case FrictionCombine::Multiply:
            return friction1 * friction2;
        default:
            return (friction1 + friction2) * 0.5f;
    }
}

float PhysicsMaterial::CombineRestitution(float restitution1, float restitution2, RestitutionCombine mode) {
    switch (mode) {
        case RestitutionCombine::Average:
            return (restitution1 + restitution2) * 0.5f;
        case RestitutionCombine::Minimum:
            return std::min(restitution1, restitution2);
        case RestitutionCombine::Maximum:
            return std::max(restitution1, restitution2);
        case RestitutionCombine::Multiply:
            return restitution1 * restitution2;
        default:
            return (restitution1 + restitution2) * 0.5f;
    }
}

PhysicsMaterial* PhysicsMaterial::GetDefault() {
    static std::unique_ptr<PhysicsMaterial> defaultMaterial = nullptr;
    if (!defaultMaterial) {
        defaultMaterial = std::make_unique<PhysicsMaterial>("Default");
        defaultMaterial->SetDynamicFriction(0.6f);
        defaultMaterial->SetStaticFriction(0.6f);
        defaultMaterial->SetRestitution(0.0f);
        defaultMaterial->SetDensity(1.0f);
    }
    return defaultMaterial.get();
}

PhysicsMaterial* PhysicsMaterial::GetBouncy() {
    static std::unique_ptr<PhysicsMaterial> bouncyMaterial = nullptr;
    if (!bouncyMaterial) {
        bouncyMaterial = std::make_unique<PhysicsMaterial>("Bouncy");
        bouncyMaterial->SetDynamicFriction(0.6f);
        bouncyMaterial->SetStaticFriction(0.6f);
        bouncyMaterial->SetRestitution(1.0f);
        bouncyMaterial->SetDensity(1.0f);
    }
    return bouncyMaterial.get();
}

PhysicsMaterial* PhysicsMaterial::GetIce() {
    static std::unique_ptr<PhysicsMaterial> iceMaterial = nullptr;
    if (!iceMaterial) {
        iceMaterial = std::make_unique<PhysicsMaterial>("Ice");
        iceMaterial->SetDynamicFriction(0.02f);
        iceMaterial->SetStaticFriction(0.02f);
        iceMaterial->SetRestitution(0.05f);
        iceMaterial->SetDensity(0.92f);
    }
    return iceMaterial.get();
}

PhysicsMaterial* PhysicsMaterial::GetRubber() {
    static std::unique_ptr<PhysicsMaterial> rubberMaterial = nullptr;
    if (!rubberMaterial) {
        rubberMaterial = std::make_unique<PhysicsMaterial>("Rubber");
        rubberMaterial->SetDynamicFriction(1.0f);
        rubberMaterial->SetStaticFriction(1.0f);
        rubberMaterial->SetRestitution(0.8f);
        rubberMaterial->SetDensity(1.52f);
    }
    return rubberMaterial.get();
}

PhysicsMaterial* PhysicsMaterial::GetMetal() {
    static std::unique_ptr<PhysicsMaterial> metalMaterial = nullptr;
    if (!metalMaterial) {
        metalMaterial = std::make_unique<PhysicsMaterial>("Metal");
        metalMaterial->SetDynamicFriction(0.4f);
        metalMaterial->SetStaticFriction(0.4f);
        metalMaterial->SetRestitution(0.05f);
        metalMaterial->SetDensity(7.8f);
    }
    return metalMaterial.get();
}

}
