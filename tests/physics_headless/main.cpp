#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <memory>


#include "Physics/PhysicsWorld.h"
#include "Physics/RigidBody/RigidBody.h"
#include "Core/Components/ColliderComponent.h"
#include "Core/Components/TransformComponent.h"

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

static bool runScenario(const ScenarioConfig& cfg, bool verbose, Metrics& out) {
    PhysicsWorld world;
    world.Initialize();


    auto groundCollider = std::make_unique<ColliderComponent>();
    groundCollider->SetBoxCollider(Vector3(100.0f, 1.0f, 100.0f));
    groundCollider->SetRestitution(cfg.restitutionCol);
    groundCollider->SetFriction(cfg.frictionCol);
    TransformComponent groundTr;
    groundTr.transform.SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    groundCollider->SetOwnerTransform(&groundTr);
    world.AddStaticCollider(groundCollider.get());

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
    const int steps = 600; // 10 seconds

    float prevVy = rb->GetVelocity().y;

    float groundTop = 0.0f + 1.0f + 0.5f; // ground half-height 1.0 + box half 0.5 = 1.5 height from ground center to top of box bottom at rest

    float minPen = 0.0f;
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
    out.minPenetration = minPen;
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

    return allPass ? 0 : 1;
}
