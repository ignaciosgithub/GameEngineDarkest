#include "Physics2DDemo.h"
#include "../PhysicsWorld2D.h"
#include "../RigidBody2D.h"
#include "../../../Core/Logging/Logger.h"

namespace GameEngine {

Physics2DDemo::Physics2DDemo() {
}

Physics2DDemo::~Physics2DDemo() {
    Shutdown();
}

void Physics2DDemo::Initialize() {
    if (m_initialized) return;
    
    Logger::Info("Initializing 2D Physics Demo...");
    
    m_physicsWorld = std::make_unique<PhysicsWorld2D>();
    m_physicsWorld->Initialize();
    
    m_physicsWorld->SetWorldBounds(Vector2(-50.0f, -50.0f), Vector2(50.0f, 50.0f));
    
    CreateBouncingBallsScene();
    
    m_initialized = true;
    Logger::Info("2D Physics Demo initialized successfully");
}

void Physics2DDemo::Shutdown() {
    if (!m_initialized) return;
    
    Logger::Info("Shutting down 2D Physics Demo...");
    
    ClearScene();
    
    if (m_physicsWorld) {
        m_physicsWorld->Shutdown();
        m_physicsWorld.reset();
    }
    
    m_initialized = false;
    Logger::Info("2D Physics Demo shutdown complete");
}

void Physics2DDemo::Update(float deltaTime) {
    if (!m_initialized || !m_physicsWorld) return;
    
    m_physicsWorld->Update(deltaTime);
}

void Physics2DDemo::CreateBouncingBallsScene() {
    ClearScene();
    Logger::Info("Creating bouncing balls scene");
    
    CreateGround();
    CreateWalls();
    
    for (int i = 0; i < 5; ++i) {
        Vector2 position(-10.0f + i * 5.0f, 10.0f + i * 2.0f);
        RigidBody2D* ball = CreateCircle(position, 1.0f, true);
        ball->SetRestitution(0.8f);
        ball->SetFriction(0.3f);
        ball->AddForce(Vector2(50.0f * (i - 2), 0.0f));
    }
}

void Physics2DDemo::CreateStackingBoxesScene() {
    ClearScene();
    Logger::Info("Creating stacking boxes scene");
    
    CreateGround();
    CreateWalls();
    
    for (int i = 0; i < 8; ++i) {
        Vector2 position(0.0f, -8.0f + i * 2.1f);
        Vector2 size(2.0f, 2.0f);
        RigidBody2D* box = CreateBox(position, size, true);
        box->SetRestitution(0.2f);
        box->SetFriction(0.7f);
    }
    
    RigidBody2D* ball = CreateCircle(Vector2(-15.0f, 5.0f), 1.5f, true);
    ball->SetRestitution(0.6f);
    ball->AddForce(Vector2(200.0f, 0.0f));
}

void Physics2DDemo::CreateMixedShapesScene() {
    ClearScene();
    Logger::Info("Creating mixed shapes scene");
    
    CreateGround();
    CreateWalls();
    
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vector2 position(-5.0f + i * 5.0f, 5.0f + j * 3.0f);
            
            if ((i + j) % 2 == 0) {
                RigidBody2D* circle = CreateCircle(position, 1.0f, true);
                circle->SetRestitution(0.7f);
                circle->SetFriction(0.4f);
            } else {
                Vector2 size(1.8f, 1.8f);
                RigidBody2D* box = CreateBox(position, size, true);
                box->SetRestitution(0.3f);
                box->SetFriction(0.6f);
            }
        }
    }
}

void Physics2DDemo::SwitchToScene(int sceneIndex) {
    m_currentScene = sceneIndex % 3;
    
    switch (m_currentScene) {
        case 0:
            CreateBouncingBallsScene();
            break;
        case 1:
            CreateStackingBoxesScene();
            break;
        case 2:
            CreateMixedShapesScene();
            break;
    }
}

void Physics2DDemo::ResetCurrentScene() {
    SwitchToScene(m_currentScene);
}

int Physics2DDemo::GetActiveBodyCount() const {
    return m_physicsWorld ? m_physicsWorld->GetActiveBodyCount() : 0;
}

int Physics2DDemo::GetCollisionCount() const {
    return m_physicsWorld ? m_physicsWorld->GetCollisionCount() : 0;
}

void Physics2DDemo::ClearScene() {
    for (auto& body : m_rigidBodies) {
        if (m_physicsWorld && body) {
            m_physicsWorld->RemoveRigidBody(body.get());
        }
    }
    m_rigidBodies.clear();
}

void Physics2DDemo::CreateGround() {
    Vector2 position(0.0f, -15.0f);
    Vector2 size(40.0f, 2.0f);
    RigidBody2D* ground = CreateBox(position, size, false);
    ground->SetBodyType(RigidBody2DType::Static);
    ground->SetRestitution(0.5f);
    ground->SetFriction(0.8f);
}

void Physics2DDemo::CreateWalls() {
    Vector2 leftPos(-20.0f, 0.0f);
    Vector2 wallSize(2.0f, 30.0f);
    RigidBody2D* leftWall = CreateBox(leftPos, wallSize, false);
    leftWall->SetBodyType(RigidBody2DType::Static);
    leftWall->SetRestitution(0.8f);
    
    Vector2 rightPos(20.0f, 0.0f);
    RigidBody2D* rightWall = CreateBox(rightPos, wallSize, false);
    rightWall->SetBodyType(RigidBody2DType::Static);
    rightWall->SetRestitution(0.8f);
}

RigidBody2D* Physics2DDemo::CreateCircle(const Vector2& position, float radius, bool isDynamic) {
    auto body = std::make_unique<RigidBody2D>();
    body->SetPosition(position);
    body->SetColliderType(Collider2DType::Circle);
    body->SetColliderRadius(radius);
    body->SetBodyType(isDynamic ? RigidBody2DType::Dynamic : RigidBody2DType::Static);
    
    if (isDynamic) {
        body->SetMass(1.0f);
        body->SetInertia(0.5f * 1.0f * radius * radius); // I = 0.5 * m * r^2 for circle
    }
    
    RigidBody2D* bodyPtr = body.get();
    m_rigidBodies.push_back(std::move(body));
    
    if (m_physicsWorld) {
        m_physicsWorld->AddRigidBody(bodyPtr);
    }
    
    return bodyPtr;
}

RigidBody2D* Physics2DDemo::CreateBox(const Vector2& position, const Vector2& size, bool isDynamic) {
    auto body = std::make_unique<RigidBody2D>();
    body->SetPosition(position);
    body->SetColliderType(Collider2DType::Box);
    body->SetColliderSize(size);
    body->SetBodyType(isDynamic ? RigidBody2DType::Dynamic : RigidBody2DType::Static);
    
    if (isDynamic) {
        body->SetMass(1.0f);
        body->SetInertia((1.0f / 12.0f) * 1.0f * (size.x * size.x + size.y * size.y));
    }
    
    RigidBody2D* bodyPtr = body.get();
    m_rigidBodies.push_back(std::move(body));
    
    if (m_physicsWorld) {
        m_physicsWorld->AddRigidBody(bodyPtr);
    }
    
    return bodyPtr;
}

}
