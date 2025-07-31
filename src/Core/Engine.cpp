#include "../Rendering/Core/OpenGLHeaders.h"
#include "Engine.h"
#include "ECS/World.h"
#include "Platform/Window.h"
#include "Platform/Input.h"
#include "Logging/Logger.h"
#include "Time/Timer.h"
#include "Editor/PlayModeManager.h"
#include "Components/TransformComponent.h"
#include "Components/CameraComponent.h"
#include "Components/MovementComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Systems/CameraSystem.h"
#include "Systems/MovementSystem.h"
#include "Scenes/TestSceneManager.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/RenderManager.h"
#include "../Rendering/Meshes/Mesh.h"
#include "../Physics/PhysicsWorld.h"
#include "../UI/EngineUI.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace GameEngine {

Engine::Engine() = default;
Engine::~Engine() = default;

bool Engine::Initialize(const std::string& title, int /*width*/, int /*height*/) {
    Logger::Initialize("engine.log", LogLevel::Debug);
    Logger::Info("Initializing Game Engine...");

    glfwSetErrorCallback([](int error, const char* description) {
        Logger::Error("GLFW Error " + std::to_string(error) + ": " + std::string(description));
    });
    
    if (!glfwInit()) {
        Logger::Error("Failed to initialize GLFW");
        return false;
    }
    
    Logger::Info("GLFW initialized successfully");

    m_window = std::make_unique<Window>();
    if (!m_window->Create(title, 1280, 720)) {
        Logger::Error("Failed to create window");
        return false;
    }

    Logger::Info("OpenGL context created");

    m_world = std::make_unique<World>();

    m_inputManager = std::make_unique<InputManager>();
    m_inputManager->Initialize();
    Logger::Info("Input manager initialized");

    m_renderer = Renderer::Create();
    if (!m_renderer || !m_renderer->Initialize()) {
        Logger::Error("Failed to initialize renderer");
        return false;
    }

    m_renderManager = std::make_unique<RenderManager>();
    if (!m_renderManager->Initialize(1280, 720)) {
        Logger::Error("Failed to initialize render manager");
        return false;
    }
    Logger::Info("Render manager initialized with multiple pipelines");

    m_physicsWorld = std::make_unique<PhysicsWorld>();

    m_engineUI = std::make_unique<EngineUI>();
    if (!m_engineUI->Initialize(m_window->GetGLFWWindow())) {
        Logger::Error("Failed to initialize Engine UI");
        return false;
    }
    Logger::Info("Engine UI initialized successfully");

    m_playModeManager = std::make_unique<PlayModeManager>();
    m_playModeManager->Initialize(m_world.get(), m_window.get());
    m_engineUI->SetPlayModeManager(m_playModeManager.get());
    Logger::Info("Play Mode Manager initialized successfully");

    m_world->AddSystem<CameraSystem>();
    m_world->AddSystem<MovementSystem>(m_inputManager.get(), m_window.get());

    m_testSceneManager = std::make_unique<TestSceneManager>(m_world.get(), m_renderManager.get());
    Logger::Info("Test scene manager initialized");

    CreateDemoScene();
    
    m_testSceneManager->LoadScene(TestSceneType::MultipleLight);
    m_testSceneManager->SwitchRenderingPipeline(RenderPipelineType::Forward);
    Logger::Info("Loaded MultipleLight test scene with 5x5 cube grid using Forward rendering");

    Timer::Initialize();
    Logger::Info("Timer system initialized");

    m_isRunning = true;

    Logger::Info("Game Engine initialized successfully");
    return true;
}

void Engine::Run() {
    Logger::Info("Starting main loop...");
    
    while (m_isRunning && !m_window->ShouldClose()) {
        Timer::Update();
        float deltaTime = Timer::GetDeltaTime();

        m_window->PollEvents();
        
        Update(deltaTime);
        Render();
        
        m_window->SwapBuffers();
    }
}

void Engine::Update(float deltaTime) {
    if (m_inputManager) {
        m_inputManager->Update();
    }

    if (m_world) {
        m_world->Update(deltaTime);
    }

    if (m_physicsWorld) {
        m_physicsWorld->Update(deltaTime);
    }

    if (m_playModeManager) {
        m_playModeManager->Update(deltaTime);
    }

    if (m_engineUI) {
        m_engineUI->Update(m_world.get(), deltaTime);
    }

    if (m_inputManager && m_inputManager->IsKeyPressed(KeyCode::Escape)) {
        m_isRunning = false;
    }

    if (m_testSceneManager) {
        if (m_inputManager->IsKeyPressed(KeyCode::Key1)) {
            m_testSceneManager->LoadScene(TestSceneType::BasicLighting);
            m_testSceneManager->SwitchRenderingPipeline(RenderPipelineType::Deferred);
        }
        if (m_inputManager->IsKeyPressed(KeyCode::Key2)) {
            m_testSceneManager->LoadScene(TestSceneType::MultipleLight);
            m_testSceneManager->SwitchRenderingPipeline(RenderPipelineType::Forward);
        }
        if (m_inputManager->IsKeyPressed(KeyCode::Key3)) {
            m_testSceneManager->LoadScene(TestSceneType::PBRMaterials);
            m_testSceneManager->SwitchRenderingPipeline(RenderPipelineType::Deferred);
        }
        if (m_inputManager->IsKeyPressed(KeyCode::Key4)) {
            m_testSceneManager->LoadScene(TestSceneType::PostProcessing);
            m_testSceneManager->SwitchRenderingPipeline(RenderPipelineType::Forward);
        }
        if (m_inputManager->IsKeyPressed(KeyCode::Key5)) {
            m_testSceneManager->LoadScene(TestSceneType::Raytracing);
            m_testSceneManager->SwitchRenderingPipeline(RenderPipelineType::Raytracing);
        }
        
        m_testSceneManager->Update(deltaTime);
    }
}

void Engine::Render() {
    if (!m_renderManager) return;

    int width, height;
    width = 1280; height = 720; // Simplified for demo
    
    Entity cameraEntity;
    for (const auto& entity : m_world->GetEntities()) {
        if (m_world->HasComponent<CameraComponent>(entity)) {
            cameraEntity = entity;
            break;
        }
    }

    RenderData renderData;
    renderData.viewportWidth = width;
    renderData.viewportHeight = height;
    
    if (cameraEntity.IsValid()) {
        auto* camera = m_world->GetComponent<CameraComponent>(cameraEntity);
        auto* transform = m_world->GetComponent<TransformComponent>(cameraEntity);

        if (camera && transform) {
            float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
            renderData.projectionMatrix = camera->GetProjectionMatrix(aspectRatio);
            renderData.viewMatrix = camera->GetViewMatrix(transform->transform);
        }
    }

    m_renderManager->BeginFrame(renderData);
    m_renderManager->Render(m_world.get());
    m_renderManager->EndFrame();

    if (m_engineUI) {
        m_engineUI->Render();
    }
}

void Engine::CreateDemoScene() {
    Logger::Info("Creating demo scene...");

    Entity cameraEntity = m_world->CreateEntity();
    m_world->AddComponent<TransformComponent>(cameraEntity, Vector3(0, 20, 10));
    
    
    m_world->AddComponent<CameraComponent>(cameraEntity, 60.0f);  // Wider FOV to see more cubes
    m_world->AddComponent<MovementComponent>(cameraEntity, 5.0f, 2.0f);

    Logger::Info("Created camera entity at position (0, 20, 10) using LookAt to view cube grid: " + std::to_string(cameraEntity.GetID()));

    for (int i = 0; i < 3; ++i) {
        Entity physicsEntity = m_world->CreateEntity();
        m_world->AddComponent<TransformComponent>(physicsEntity, Vector3(i * 2.0f, 10.0f, 0.0f));

        auto* rigidBodyComp = m_world->AddComponent<RigidBodyComponent>(physicsEntity);
        if (rigidBodyComp) {
            RigidBody* rigidBody = rigidBodyComp->GetRigidBody();
            rigidBody->SetPosition(Vector3(i * 2.0f, 10.0f, 0.0f));
            rigidBody->SetColliderType(ColliderType::Box);
            rigidBody->SetColliderSize(Vector3(1.0f, 1.0f, 1.0f));
            rigidBody->SetMass(1.0f);

            m_physicsWorld->AddRigidBody(rigidBody);
        }

        Logger::Info("Created physics entity: " + std::to_string(physicsEntity.GetID()));
    }

    Entity groundEntity = m_world->CreateEntity();
    m_world->AddComponent<TransformComponent>(groundEntity, Vector3(0, -1, 0));

    auto* groundRigidBodyComp = m_world->AddComponent<RigidBodyComponent>(groundEntity);
    if (groundRigidBodyComp) {
        RigidBody* groundRigidBody = groundRigidBodyComp->GetRigidBody();
        groundRigidBody->SetPosition(Vector3(0, -1, 0));
        groundRigidBody->SetColliderType(ColliderType::Box);
        groundRigidBody->SetColliderSize(Vector3(20.0f, 1.0f, 20.0f));
        groundRigidBody->SetBodyType(RigidBodyType::Static);

        m_physicsWorld->AddRigidBody(groundRigidBody);
    }

    Logger::Info("Created ground entity: " + std::to_string(groundEntity.GetID()));
    Logger::Info("Demo scene created successfully");
}

void Engine::Shutdown() {
    Logger::Info("Shutting down Game Engine...");
    
    m_engineUI.reset();
    if (m_testSceneManager) {
        m_testSceneManager.reset();
    }
    if (m_renderManager) {
        m_renderManager->Shutdown();
        m_renderManager.reset();
    }
    m_physicsWorld.reset();
    m_renderer.reset();
    m_inputManager.reset();
    m_world.reset();
    m_window.reset();
    
    glfwTerminate();
    
    Logger::Info("Game Engine shutdown complete");
    Logger::Shutdown();
}

}
