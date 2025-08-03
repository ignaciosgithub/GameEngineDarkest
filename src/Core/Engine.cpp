#include "../Rendering/Core/OpenGLHeaders.h"
#include "Engine.h"
#include "ECS/World.h"
#include "Platform/Window.h"
#include "Platform/Input.h"
#include "Logging/Logger.h"
#include "Time/Timer.h"
#include "Editor/PlayModeManager.h"
#include "Editor/SelectionManager.h"
#include "Components/TransformComponent.h"
#include "Components/CameraComponent.h"
#include "Components/MovementComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/MeshComponent.h"
#include "Systems/CameraSystem.h"
#include "Systems/MovementSystem.h"
#include "Systems/PhysicsSystem.h"
#include "Scenes/TestSceneManager.h"
#include "Scripting/External/ExternalScriptManager.h"
#include "Project/ProjectManager.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/RenderManager.h"
#include "../Rendering/Meshes/Mesh.h"
#include "../Rendering/Lighting/Light.h"
#include "../Rendering/Debug/DebugRenderer.h"
#include "../Physics/PhysicsWorld.h"
#include "../UI/EngineUI.h"
#include "../UI/Panels/ViewportPanel.h"
#include <cmath>
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

    m_window->SetKeyCallback([this](int key, int scancode, int action, int mods) {
        if (m_inputManager) {
            m_inputManager->OnKeyEventThreaded(key, scancode, action, mods);
        }
    });
    
    m_window->SetMouseButtonCallback([this](int button, int action, int mods) {
        if (m_inputManager) {
            m_inputManager->OnMouseButtonEventThreaded(button, action, mods);
        }
    });
    
    m_window->SetMouseMoveCallback([this](double xpos, double ypos) {
        if (m_inputManager) {
            m_inputManager->OnMouseMoveEventThreaded(xpos, ypos);
        }
    });
    Logger::Info("Window callbacks connected to input manager");

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
    
    DebugRenderer::Initialize();
    Logger::Info("Debug renderer initialized for gizmo rendering");

    m_physicsWorld = std::make_unique<PhysicsWorld>(m_world.get());
    m_physicsWorld->Initialize();
    m_world->SetPhysicsWorld(m_physicsWorld.get());

    m_engineUI = std::make_unique<EngineUI>();
    if (!m_engineUI->Initialize(m_window->GetGLFWWindow())) {
        Logger::Error("Failed to initialize Engine UI");
        return false;
    }
    Logger::Info("Engine UI initialized successfully");

    m_playModeManager = std::make_unique<PlayModeManager>();
    m_playModeManager->Initialize(m_world.get(), m_window.get());
    
    m_selectionManager = std::make_unique<SelectionManager>();
    
    m_engineUI->SetPlayModeManager(m_playModeManager.get());
    m_engineUI->SetPhysicsWorld(m_physicsWorld.get());
    m_engineUI->SetSelectionManager(m_selectionManager.get());
    Logger::Info("Play Mode Manager and Selection Manager initialized successfully");

    m_world->AddSystem<CameraSystem>();
    m_world->AddSystem<MovementSystem>(m_inputManager.get(), m_window.get(), m_playModeManager.get());
    m_world->AddSystem<PhysicsSystem>(m_playModeManager.get(), m_physicsWorld.get());

    m_testSceneManager = std::make_unique<TestSceneManager>(m_world.get(), m_renderManager.get());
    Logger::Info("Test scene manager initialized");

    ExternalScriptManager::Instance().Initialize();
    Logger::Info("External script manager initialized");

    if (!ProjectManager::Instance().IsProjectLoaded()) {
        std::string defaultProjectPath = "DefaultProject";
        if (ProjectManager::Instance().CreateProject(defaultProjectPath, "Default Project")) {
            Logger::Info("Created default project for JSON saving functionality");
        } else {
            Logger::Warning("Failed to create default project - JSON saving may not work");
        }
    }

    CreateDemoScene();
    
    // m_testSceneManager->LoadScene(TestSceneType::MultipleLight);
    // m_testSceneManager->SwitchRenderingPipeline(RenderPipelineType::Forward);
    // Logger::Info("Loaded MultipleLight test scene with 5x5 cube grid using Forward rendering");

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

    if (m_physicsWorld && m_playModeManager && m_playModeManager->IsInPlayMode()) {
        m_physicsWorld->Update(deltaTime);
        if (auto* physicsSystem = m_world->GetSystem<PhysicsSystem>()) {
            physicsSystem->OnUpdate(m_world.get(), deltaTime);
        }
    }

    if (m_playModeManager) {
        m_playModeManager->Update(deltaTime);
    }
    
    if (m_selectionManager) {
        m_selectionManager->Update(m_world.get());
    }

    ExternalScriptManager::Instance().CheckForScriptChanges();
    ExternalScriptManager::Instance().ExecuteUpdateScripts(m_world.get(), deltaTime);

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
    
    if (m_selectionManager && m_selectionManager->HasSelection() && m_playModeManager && !m_playModeManager->IsInPlayMode()) {
        Entity selectedEntity = m_selectionManager->GetSelectedEntity();
        if (selectedEntity.IsValid()) {
            auto* transform = m_world->GetComponent<TransformComponent>(selectedEntity);
            auto* mesh = m_world->GetComponent<MeshComponent>(selectedEntity);
            
            if (transform) {
                Vector3 position = transform->transform.GetPosition();
                Vector3 objectSize = Vector3(1.0f, 1.0f, 1.0f); // Default size
                
                if (mesh && mesh->HasMesh()) {
                    objectSize = Vector3(2.0f, 2.0f, 2.0f);
                }
                
                DebugRenderer::RenderSelectionOutline(position, objectSize, Vector3(1.0f, 1.0f, 0.0f));
                
                DebugRenderer::RenderMovementGizmo(position, objectSize);
            }
        }
    }
    
    m_renderManager->EndFrame();

    if (m_engineUI) {
        ViewportPanel* viewportPanel = m_engineUI->GetViewportPanel();
        if (viewportPanel && m_renderManager) {
            auto currentPipeline = m_renderManager->GetCurrentPipeline();
            if (currentPipeline) {
                viewportPanel->SetFramebuffer(currentPipeline->GetFramebuffer());
            }
        }
        m_engineUI->Render();
    }
}

void Engine::CreateDemoScene() {
    Logger::Info("Creating simplified default scene...");

    Entity cameraEntity = m_world->CreateEntity();
    m_world->AddComponent<TransformComponent>(cameraEntity, Vector3(0, 5, 10));
    
    auto* cameraTransform = m_world->GetComponent<TransformComponent>(cameraEntity);
    if (cameraTransform) {
        Vector3 direction = (Vector3(0, 0, 0) - Vector3(0, 5, 10)).Normalized();
        float pitch = std::asin(-direction.y);
        float yaw = std::atan2(direction.x, direction.z);
        cameraTransform->transform.SetRotation(Quaternion::FromEulerAngles(pitch, yaw, 0.0f));
    }
    
    m_world->AddComponent<CameraComponent>(cameraEntity, 60.0f);
    m_world->AddComponent<MovementComponent>(cameraEntity, 5.0f, 2.0f);
    Logger::Info("Created camera entity at position (0, 5, 10): " + std::to_string(cameraEntity.GetID()));

    Entity cubeEntity = m_world->CreateEntity();
    m_world->AddComponent<TransformComponent>(cubeEntity, Vector3(0, 0, 0));
    auto* cubeTransform = m_world->GetComponent<TransformComponent>(cubeEntity);
    if (cubeTransform) {
        cubeTransform->transform.SetRotation(Quaternion::FromEulerAngles(0.3f, 0.5f, 0.2f));
    }
    m_world->AddComponent<MeshComponent>(cubeEntity, "cube");
    Logger::Info("Created cube entity at origin with rotation: " + std::to_string(cubeEntity.GetID()));

    Entity lightEntity = m_world->CreateEntity();
    m_world->AddComponent<TransformComponent>(lightEntity, Vector3(0, 3, 0));
    auto* lightComp = m_world->AddComponent<LightComponent>(lightEntity, LightType::Point);
    lightComp->light.SetPosition(Vector3(0, 3, 0));
    lightComp->light.SetColor(Vector3(1.0f, 1.0f, 1.0f));
    lightComp->light.SetIntensity(2.0f);
    lightComp->light.SetRange(10.0f);
    Logger::Info("Created point light at position (0, 3, 0): " + std::to_string(lightEntity.GetID()));

    Logger::Info("Simplified default scene created successfully");
}

void Engine::Shutdown() {
    Logger::Info("Shutting down Game Engine...");
    
    DebugRenderer::Shutdown();
    ExternalScriptManager::Instance().Shutdown();
    
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
