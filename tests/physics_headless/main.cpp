#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <memory>


#include "Physics/PhysicsWorld.h"
#include "Physics/RigidBody/RigidBody.h"
#include "Core/Components/ColliderComponent.h"
#include "Core/Components/TransformComponent.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector3.h"

using namespace GameEngine;

struct ScenarioConfig {
    float restitutionRB = 0.0f;
    float restitutionCol = 0.0f;
    float frictionRB = 0.5f;
    float frictionCol = 0.8f;
    float damping = 0.05f;
    float gravityY = -9.81f;
    float startY = 10.0f;
    float initialVelY = 0.0f;
    std::string name = "rest0";
};

struct Metrics {
    float finalY = 0.0f;
    float finalVy = 0.0f;
    float minPenetration = 0.0f;
    int bounceCount = 0;
    bool contactEver = false;
};

static float GroundTopY() {
    return 0.0f + 1.0f; // ground center at 0 with half-height 1
}

static void SetupGroundStaticCollider(PhysicsWorld& world, float restitution, float friction, TransformComponent& groundTr, std::unique_ptr<ColliderComponent>& groundCollider) {
    groundCollider = std::make_unique<ColliderComponent>();
    groundCollider->SetBoxCollider(Vector3(100.0f, 1.0f, 100.0f));
    groundCollider->SetRestitution(restitution);
    groundCollider->SetFriction(friction);
    groundTr.transform.SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    groundCollider->SetOwnerTransform(&groundTr);
    world.AddStaticCollider(groundCollider.get());
}

static void SetupGroundStaticRigidBody(PhysicsWorld& world, float restitution, float friction, TransformComponent& groundTr, std::unique_ptr<RigidBody>& groundRB, std::unique_ptr<ColliderComponent>& groundCollider) {
    groundRB = std::make_unique<RigidBody>();
    groundRB->SetBodyType(RigidBodyType::Static);
    groundRB->SetMass(0.0f);
    groundRB->SetRestitution(restitution);
    groundRB->SetFriction(friction);
    groundRB->SetPosition(Vector3(0.0f, 0.0f, 0.0f));

    groundCollider = std::make_unique<ColliderComponent>();
    groundCollider->SetBoxCollider(Vector3(100.0f, 1.0f, 100.0f));
    groundCollider->SetRestitution(restitution);
    groundCollider->SetFriction(friction);

    groundTr.transform.SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    groundCollider->SetOwnerTransform(&groundTr);

    groundRB->SetColliderComponent(groundCollider.get());
    groundRB->SetTransformComponent(&groundTr);
    world.AddRigidBody(groundRB.get());
}

static bool runScenario(const ScenarioConfig& cfg, bool verbose, Metrics& out) {
    PhysicsWorld world;
    world.Initialize();

    TransformComponent groundTr;
    std::unique_ptr<ColliderComponent> groundCollider;
    SetupGroundStaticCollider(world, cfg.restitutionCol, cfg.frictionCol, groundTr, groundCollider);

    auto rb = std::make_unique<RigidBody>();
    rb->SetBodyType(RigidBodyType::Dynamic);
    rb->SetMass(1.0f);
    rb->SetDamping(cfg.damping);
    rb->SetRestitution(cfg.restitutionRB);
    rb->SetFriction(cfg.frictionRB);
    rb->SetPosition(Vector3(0.0f, cfg.startY, 0.0f));
    rb->SetVelocity(Vector3(0.0f, cfg.initialVelY, 0.0f));

    auto boxCol = std::make_unique<ColliderComponent>();
    boxCol->SetBoxCollider(Vector3(0.5f, 0.5f, 0.5f));
    TransformComponent boxTr;
    boxTr.transform.SetPosition(Vector3(0.0f, cfg.startY, 0.0f));
    boxCol->SetOwnerTransform(&boxTr);
    boxCol->SetRestitution(cfg.restitutionRB);
    boxCol->SetFriction(cfg.frictionRB);

    rb->SetColliderComponent(boxCol.get());

    world.AddRigidBody(rb.get());

    const float dt = 1.0f / 60.0f;
    const int steps = 600;

    float prevVy = rb->GetVelocity().y;

    float groundTop = GroundTopY() + 0.5f; // add box half to compare against its center y

    int bounces = 0;
    bool inContact = false;

    for (int i = 0; i < steps; ++i) {
        world.Update(dt);

        boxTr.transform.SetPosition(rb->GetPosition());

        float y = rb->GetPosition().y;
        float vy = rb->GetVelocity().y;

        if (y <= groundTop + 0.01f) {
            out.contactEver = true;
            inContact = true;
        }

        if ((prevVy < -1e-3f && vy > 1e-3f) || (prevVy > 1e-3f && vy < -1e-3f)) {
            if (inContact) bounces++;
        }

        prevVy = vy;
    }

    out.finalY = rb->GetPosition().y;
    out.finalVy = rb->GetVelocity().y;
    out.minPenetration = 0.0f;
    out.bounceCount = bounces;

    bool pass = true;
    if (cfg.restitutionRB == 0.0f && cfg.restitutionCol == 0.0f) {
        if (std::fabs(out.finalVy) > 0.02f) pass = false;
        if (out.finalY < groundTop - 0.02f) pass = false;
        if (out.bounceCount > 0) pass = false;
        if (!out.contactEver) pass = false;
    }

    if (verbose) {
        std::cout << "Scenario " << cfg.name << ": finalY=" << out.finalY
                  << " finalVy=" << out.finalVy
                  << " bounces=" << out.bounceCount
                  << " contactEver=" << (out.contactEver ? "yes" : "no")
                  << " pass=" << (pass ? "yes" : "no") << std::endl;
    }

    return pass;
}

static bool runRotatedBounceScenario(const std::string& name, float restitution, bool verbose, bool& outContact, int& outBounces) {
    PhysicsWorld world;
    world.Initialize();

    TransformComponent groundTr;
    std::unique_ptr<ColliderComponent> groundCollider;
    SetupGroundStaticCollider(world, restitution, 0.8f, groundTr, groundCollider);

    auto rb = std::make_unique<RigidBody>();
    rb->SetBodyType(RigidBodyType::Dynamic);
    rb->SetMass(1.0f);
    rb->SetDamping(0.05f);
    rb->SetRestitution(restitution);
    rb->SetFriction(0.5f);
    rb->SetPosition(Vector3(0.1f, 5.0f, 0.0f)); // slight lateral offset to bias contact
    rb->SetVelocity(Vector3(0.0f, 0.0f, 0.0f));

    auto boxCol = std::make_unique<ColliderComponent>();
    boxCol->SetBoxCollider(Vector3(0.5f, 0.5f, 0.5f));
    TransformComponent boxTr;
    Quaternion initialRot = Quaternion::FromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), 0.15f);
    boxTr.transform.SetPosition(rb->GetPosition());
    boxTr.transform.SetRotation(initialRot);
    boxCol->SetOwnerTransform(&boxTr);
    boxCol->SetRestitution(restitution);
    boxCol->SetFriction(0.5f);

    rb->SetColliderComponent(boxCol.get());
    rb->SetRotation(initialRot);

    world.AddRigidBody(rb.get());

    const float dt = 1.0f / 60.0f;
    const int steps = 900;

    float groundTop = GroundTopY() + 0.5f;

    bool contactEver = false;
    int bounces = 0;

    float prevVy = 0.0f;
    float lastPeak = -1e9f;
    bool energyDecreasing = true;
    float lastPeakHeight = 0.0f;

    for (int i = 0; i < steps; ++i) {
        world.Update(dt);

        boxTr.transform.SetPosition(rb->GetPosition());
        boxTr.transform.SetRotation(rb->GetRotation());

        float y = rb->GetPosition().y;
        float vy = rb->GetVelocity().y;

        if (y <= groundTop + 0.01f) contactEver = true;

        if (prevVy > 0.0f && vy <= 0.0f) {
            if (lastPeak > -1e8f) {
                if (y > lastPeakHeight - 1e-4f) {
                    energyDecreasing = false;
                }
            }
            lastPeak = y;
            lastPeakHeight = y;
        }

        if (prevVy < -1e-3f && vy > 1e-3f && (y <= groundTop + 0.05f)) {
            bounces++;
        }

        prevVy = vy;
    }

    if (verbose) {
        std::cout << "Scenario " << name << ": contactEver=" << (contactEver ? "yes" : "no")
                  << " bounces=" << bounces
                  << " finalVy=" << rb->GetVelocity().y
                  << std::endl;
    }

    outContact = contactEver;
    outBounces = bounces;

    if (restitution == 0.0f) {
        return contactEver && bounces == 0 && std::fabs(rb->GetVelocity().y) < 0.05f;
    } else {
        return contactEver && bounces > 0 && energyDecreasing;
    }
}

static bool runTorqueFromContactScenario(bool verbose) {
    PhysicsWorld world;
    world.Initialize();

    TransformComponent groundTr;
    std::unique_ptr<ColliderComponent> groundCollider;
    SetupGroundStaticCollider(world, 0.1f, 0.8f, groundTr, groundCollider);

    auto rb = std::make_unique<RigidBody>();
    rb->SetBodyType(RigidBodyType::Dynamic);
    rb->SetMass(1.0f);
    rb->SetDamping(0.01f);
    rb->SetRestitution(0.1f);
    rb->SetFriction(0.6f);
    rb->SetPosition(Vector3(0.3f, 3.0f, 0.0f));
    rb->SetVelocity(Vector3(0.0f, 0.0f, 0.0f));

    auto boxCol = std::make_unique<ColliderComponent>();
    boxCol->SetBoxCollider(Vector3(0.5f, 0.5f, 0.5f));
    TransformComponent boxTr;
    Quaternion initialRot = Quaternion::FromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), 0.2f);
    boxTr.transform.SetPosition(rb->GetPosition());
    boxTr.transform.SetRotation(initialRot);
    boxCol->SetOwnerTransform(&boxTr);

    rb->SetColliderComponent(boxCol.get());
    rb->SetRotation(initialRot);

    world.AddRigidBody(rb.get());

    const float dt = 1.0f / 60.0f;
    const int steps = 480;

    bool hadContact = false;
    float angVelMagMax = 0.0f;

    for (int i = 0; i < steps; ++i) {
        world.Update(dt);

        boxTr.transform.SetPosition(rb->GetPosition());
        boxTr.transform.SetRotation(rb->GetRotation());

        float y = rb->GetPosition().y;
        if (y <= GroundTopY() + 0.55f) hadContact = true;

        float angMag = rb->GetAngularVelocity().Length();
        if (angMag > angVelMagMax) angVelMagMax = angMag;
    }

    Quaternion q = rb->GetRotation();
    float len = std::sqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    bool quatValid = std::isfinite(len) && std::fabs(len - 1.0f) < 0.05f;

    if (verbose) {
        std::cout << "Torque scenario: hadContact=" << (hadContact ? "yes" : "no")
                  << " maxAngVel=" << angVelMagMax
                  << " quatValid=" << (quatValid ? "yes" : "no")
                  << std::endl;
    }

    return hadContact && angVelMagMax > 1e-3f && quatValid;
}

static bool runStaticParityScenario(bool verbose) {
    bool contact1 = false;
    int b1 = 0;

    bool pass1 = runRotatedBounceScenario("static_collider_zero", 0.0f, verbose, contact1, b1);

    PhysicsWorld world;
    world.Initialize();

    TransformComponent groundTr;
    std::unique_ptr<RigidBody> groundRB;
    std::unique_ptr<ColliderComponent> groundCol;
    SetupGroundStaticRigidBody(world, 0.0f, 0.8f, groundTr, groundRB, groundCol);

    auto rb = std::make_unique<RigidBody>();
    rb->SetBodyType(RigidBodyType::Dynamic);
    rb->SetMass(1.0f);
    rb->SetDamping(0.05f);
    rb->SetRestitution(0.0f);
    rb->SetFriction(0.5f);
    rb->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
    rb->SetVelocity(Vector3(0.0f, 0.0f, 0.0f));

    auto boxCol = std::make_unique<ColliderComponent>();
    boxCol->SetBoxCollider(Vector3(0.5f, 0.5f, 0.5f));
    TransformComponent boxTr;
    boxTr.transform.SetPosition(rb->GetPosition());
    boxTr.transform.SetRotation(Quaternion::FromAxisAngle(Vector3(0,0,1), 0.15f));
    boxCol->SetOwnerTransform(&boxTr);
    rb->SetColliderComponent(boxCol.get());
    rb->SetRotation(Quaternion::FromAxisAngle(Vector3(0,0,1), 0.15f));
    rb->SetTransformComponent(&boxTr);

    world.AddRigidBody(rb.get());

    const float dt = 1.0f/60.0f;
    const int steps = 600;
    int bounces = 0;
    bool contactEver = false;
    float prevVy = 0.0f;
    float groundTop = GroundTopY() + 0.5f;

    for (int i = 0; i < steps; ++i) {
        world.Update(dt);
        boxTr.transform.SetPosition(rb->GetPosition());
        boxTr.transform.SetRotation(rb->GetRotation());

        float y = rb->GetPosition().y;
        float vy = rb->GetVelocity().y;

        if (y <= groundTop + 0.01f) contactEver = true;
        if ((prevVy < -1e-3f && vy > 1e-3f) && (y <= groundTop + 0.05f)) bounces++;
        prevVy = vy;
    }

    if (verbose) {
        std::cout << "Static RB details: bounces=" << bounces
                  << " finalVy=" << rb->GetVelocity().y
                  << " contactEver=" << (contactEver ? "yes" : "no")
                  << std::endl;
    }

    bool pass2 = contactEver && bounces == 0 && std::fabs(rb->GetVelocity().y) < 0.05f;

    if (verbose) {
        std::cout << "Static parity: colliderOnly pass=" << (pass1 ? "yes" : "no")
                  << " staticRB pass=" << (pass2 ? "yes" : "no")
                  << std::endl;
    }

    return pass1 && pass2;
}
static bool runAngularDampingDecayTest(bool verbose) {
    PhysicsWorld world;
    world.Initialize();
    TransformComponent groundTr;
    std::unique_ptr<ColliderComponent> groundCollider;
    SetupGroundStaticCollider(world, 0.0f, 0.9f, groundTr, groundCollider);

    auto rb = std::make_unique<RigidBody>();
    rb->SetBodyType(RigidBodyType::Dynamic);
    rb->SetMass(1.0f);
    rb->SetDamping(0.01f);
    rb->SetAngularDamping(0.2f);
    rb->SetRestitution(0.0f);
    rb->SetFriction(0.9f);
    rb->SetPosition(Vector3(0.0f, 3.0f, 0.0f));

    auto boxCol = std::make_unique<ColliderComponent>();
    boxCol->SetBoxCollider(Vector3(0.5f, 0.5f, 0.5f));
    TransformComponent boxTr;
    boxTr.transform.SetPosition(rb->GetPosition());
    boxCol->SetOwnerTransform(&boxTr);
    rb->SetColliderComponent(boxCol.get());

    world.AddRigidBody(rb.get());

    rb->SetAngularVelocity(Vector3(0.0f, 5.0f, 0.0f));
    const float dt = 1.0f / 120.0f;
    const int steps = 360;
    for (int i = 0; i < steps; ++i) {
        world.Update(dt);
        boxTr.transform.SetPosition(rb->GetPosition());
    }
    float omega = rb->GetAngularVelocity().Length();
    if (verbose) {
        std::cout << "AngularDampingDecay: omega=" << omega << std::endl;
    }
    world.Shutdown();
    return omega < 5.0f;
}

static bool runFlatDropNoRotationTest(bool verbose) {
    PhysicsWorld world;
    world.Initialize();
    TransformComponent groundTr;
    std::unique_ptr<ColliderComponent> groundCollider;
    SetupGroundStaticCollider(world, 0.0f, 0.9f, groundTr, groundCollider);

    auto rb = std::make_unique<RigidBody>();
    rb->SetBodyType(RigidBodyType::Dynamic);
    rb->SetMass(1.0f);
    rb->SetDamping(0.01f);
    rb->SetRestitution(0.0f);
    rb->SetFriction(0.9f);
    rb->SetPosition(Vector3(0.0f, 3.0f, 0.0f));
    rb->SetVelocity(Vector3(0.0f, 0.0f, 0.0f));
    rb->SetAngularVelocity(Vector3(0.0f, 0.0f, 0.0f));

    auto boxCol = std::make_unique<ColliderComponent>();
    boxCol->SetBoxCollider(Vector3(0.5f, 0.5f, 0.5f));
    TransformComponent boxTr;
    boxTr.transform.SetPosition(rb->GetPosition());
    boxCol->SetOwnerTransform(&boxTr);
    rb->SetColliderComponent(boxCol.get());

    world.AddRigidBody(rb.get());

    const float dt = 1.0f / 120.0f;
    const int steps = 480;
    float maxAng = 0.0f;
    for (int i = 0; i < steps; ++i) {
        world.Update(dt);
        boxTr.transform.SetPosition(rb->GetPosition());
        float ang = rb->GetAngularVelocity().Length();
        if (ang > maxAng) maxAng = ang;
    }
    float finalAng = rb->GetAngularVelocity().Length();
    if (verbose) {
        std::cout << "FlatDropNoRotation: maxAng=" << maxAng << " finalAng=" << finalAng << std::endl;
    }
    world.Shutdown();
    return finalAng < 0.02f;
}

int main(int argc, char** argv) { (void)argc; (void)argv;
    bool verbose = true;

    std::vector<ScenarioConfig> scenarios;
    scenarios.push_back(ScenarioConfig{0.0f, 0.0f, 0.5f, 0.8f, 0.05f, -9.81f, 10.0f, 0.0f, "rest0"});
    scenarios.push_back(ScenarioConfig{0.5f, 0.5f, 0.5f, 0.8f, 0.05f, -9.81f, 10.0f, 0.0f, "rest05"});
    scenarios.push_back(ScenarioConfig{1.0f, 1.0f, 0.5f, 0.8f, 0.05f, -9.81f, 10.0f, 0.0f, "rest1"});

    bool allPass = true;
    for (auto& sc : scenarios) {
        Metrics m;
        bool pass = runScenario(sc, verbose, m);
        if (!pass) allPass = false;
    }

    bool contact;
    int bounces;
    bool passRot0 = runRotatedBounceScenario("rot_zero_rest", 0.0f, verbose, contact, bounces);
    if (!passRot0) allPass = false;
    bool passRotSmall = runRotatedBounceScenario("rot_small_rest", 0.15f, verbose, contact, bounces);
    if (!passRotSmall) allPass = false;

    bool passTorque = runTorqueFromContactScenario(verbose);
    if (!passTorque) allPass = false;

    bool passStaticParity = runStaticParityScenario(verbose);
    if (!passStaticParity) allPass = false;

    bool passAngDamp = runAngularDampingDecayTest(verbose);
    if (!passAngDamp) allPass = false;

    bool passFlatNoRot = runFlatDropNoRotationTest(verbose);
    if (!passFlatNoRot) allPass = false;

    return allPass ? 0 : 1;
}
