# GameEngineDarkest - Game Development Tutorial

## Table of Contents
1. [Getting Started](#getting-started)
2. [Engine Architecture Overview](#engine-architecture-overview)
3. [Creating Your First Game](#creating-your-first-game)
4. [Working with GameObjects](#working-with-gameobjects)
5. [Multi-Light System](#multi-light-system)
6. [Camera and Rendering](#camera-and-rendering)
7. [Physics System](#physics-system)
8. [Audio System](#audio-system)
9. [Networking](#networking)
10. [Editor and Play Mode](#editor-and-play-mode)
11. [Custom Scripts and Components](#custom-scripts-and-components)
12. [Advanced Features](#advanced-features)
13. [Best Practices](#best-practices)

## Getting Started

### Prerequisites
- C++20 compatible compiler (MSVC 2019+, GCC 9+, Clang 10+)
- CMake 3.20 or higher
- vcpkg (Windows) or system packages (Linux)

### Building the Engine
```bash
# Clone the repository
git clone https://github.com/ignaciosgithub/GameEngineDarkest.git
cd GameEngineDarkest

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j4  # Linux
# or
cmake --build . --config Release  # Windows
```

### Running the Demo
```bash
# From build directory
./demo/GameEngineDemo
```

## Engine Architecture Overview

GameEngineDarkest follows a modular Entity-Component-System (ECS) architecture with the following core modules:

- **Core**: ECS system, math utilities, platform abstraction
- **Rendering**: Multi-pipeline rendering system (Forward, Deferred, Raytracing)
- **Physics**: 3D and 2D physics simulation with collision detection
- **Audio**: 3D spatial audio with OpenAL backend
- **UI**: ImGui-based editor interface
- **Networking**: UDP/TCP networking for multiplayer games
- **Animation**: Skeletal animation and blend trees

## Creating Your First Game

### Basic Game Structure
```cpp
#include "Core/Engine.h"
#include "Core/Scenes/Scene.h"
#include "Core/GameObject/GameObject.h"

int main() {
    GameEngine::Engine engine;
    
    if (!engine.Initialize("My Game", 1280, 720)) {
        return -1;
    }
    
    // Create your game scene
    auto scene = std::make_shared<GameEngine::Scene>("MainScene");
    
    // Add game objects to scene
    CreateGameWorld(scene);
    
    engine.Run();
    engine.Shutdown();
    
    return 0;
}
```

### Creating a Simple Scene
```cpp
void CreateGameWorld(std::shared_ptr<GameEngine::Scene> scene) {
    auto& world = scene->GetWorld();
    
    // Create a camera
    auto cameraEntity = world.CreateEntity();
    auto camera = world.AddComponent<GameEngine::CameraComponent>(cameraEntity);
    camera->SetPosition(GameEngine::Vector3(0, 5, 10));
    camera->SetFOV(60.0f);
    
    // Create a ground plane
    auto groundEntity = world.CreateEntity();
    auto groundMesh = world.AddComponent<GameEngine::MeshComponent>(groundEntity);
    groundMesh->LoadMesh("assets/models/plane.obj");
    
    // Add physics to ground
    auto groundRigidBody = world.AddComponent<GameEngine::RigidBodyComponent>(groundEntity);
    groundRigidBody->SetStatic(true);
}
```

## Working with GameObjects

### GameObject Fundamentals
GameObjects are the primary entities in your game world. They have a position in space and can acquire different components at runtime:

```cpp
// Create a new GameObject
auto gameObject = std::make_unique<GameEngine::GameObject>(world, entity);

// Set position
gameObject->SetPosition(GameEngine::Vector3(0, 0, 0));
gameObject->SetRotation(GameEngine::Quaternion::Identity);
gameObject->SetScale(GameEngine::Vector3(1, 1, 1));

// Add components dynamically
auto meshComponent = gameObject->AddComponent<GameEngine::MeshComponent>();
auto rigidBodyComponent = gameObject->AddComponent<GameEngine::RigidBodyComponent>();
```

### Dynamic Component Management
```cpp
// Add a mesh component at runtime
if (!gameObject->HasComponent<GameEngine::MeshComponent>()) {
    auto mesh = gameObject->AddComponent<GameEngine::MeshComponent>();
    mesh->LoadMesh("assets/models/cube.obj");
}

// Modify component properties
auto rigidBody = gameObject->GetComponent<GameEngine::RigidBodyComponent>();
if (rigidBody) {
    rigidBody->SetMass(10.0f);
    rigidBody->SetRestitution(0.8f);
}

// Remove components
gameObject->RemoveComponent<GameEngine::MeshComponent>();
```

### Prefab System
```cpp
// Create a prefab for reusable objects
auto cratePrefab = std::make_shared<GameEngine::Prefab>("Crate");
cratePrefab->AddComponent<GameEngine::MeshComponent>("assets/models/crate.obj");
cratePrefab->AddComponent<GameEngine::RigidBodyComponent>(1.0f, 0.5f);

// Instantiate prefabs
auto crate1 = cratePrefab->Instantiate(world, GameEngine::Vector3(0, 5, 0));
auto crate2 = cratePrefab->Instantiate(world, GameEngine::Vector3(2, 5, 0));
```

## Multi-Light System

### Light Types and Properties
The engine supports multiple light types with hardware-accelerated rendering:

```cpp
#include "Rendering/Lighting/Light.h"

// Create a point light
auto pointLight = std::make_shared<GameEngine::Light>();
pointLight->SetType(GameEngine::LightType::Point);
pointLight->SetPosition(GameEngine::Vector3(0, 5, 0));
pointLight->SetColor(GameEngine::Vector3(1.0f, 0.8f, 0.6f)); // Warm white
pointLight->SetIntensity(2.0f);
pointLight->SetRange(10.0f);

// Create a directional light (sun)
auto sunLight = std::make_shared<GameEngine::Light>();
sunLight->SetType(GameEngine::LightType::Directional);
sunLight->SetDirection(GameEngine::Vector3(-0.3f, -1.0f, -0.3f));
sunLight->SetColor(GameEngine::Vector3(1.0f, 0.95f, 0.8f));
sunLight->SetIntensity(1.5f);
```

### Attaching Lights to GameObjects
```cpp
// Attach light to a GameObject for dynamic movement
auto lightObject = std::make_unique<GameEngine::GameObject>(world, entity);
auto lightComponent = lightObject->AddComponent<GameEngine::LightComponent>();

// Configure the light
lightComponent->GetLight()->SetType(GameEngine::LightType::Point);
lightComponent->GetLight()->SetColor(GameEngine::Vector3(0.0f, 1.0f, 0.0f)); // Green
lightComponent->GetLight()->SetIntensity(3.0f);

// Move the light (and GameObject) at runtime
lightObject->SetPosition(GameEngine::Vector3(sin(time) * 5, 3, cos(time) * 5));
```

### Light Occlusion and Shadows
```cpp
// Enable shadow mapping for a light
pointLight->SetCastShadows(true);
pointLight->SetShadowMapSize(1024); // Shadow map resolution

// The engine automatically handles light occlusion through collision geometry
// Lights cannot pass through walls or other solid objects
```

### Brightness Management
The engine implements automatic brightness clamping to prevent oversaturation:

```cpp
// Maximum brightness is automatically clamped to hardware limits
const float MAX_BRIGHTNESS = 100.0f; // Defined in Light.h

// Multiple lights accumulate up to the maximum
auto redLight = CreateLight(GameEngine::Vector3(1, 0, 0), 50.0f);
auto blueLight = CreateLight(GameEngine::Vector3(0, 0, 1), 60.0f);
// Total brightness will be clamped to MAX_BRIGHTNESS
```

## Camera and Rendering

### Camera Controls
```cpp
auto camera = gameObject->GetComponent<GameEngine::CameraComponent>();

// Dynamic FOV adjustment
camera->SetFOV(75.0f); // Set field of view (10-170 degrees)
camera->AdjustFOV(5.0f); // Increase FOV by 5 degrees
float currentFOV = camera->GetFOV();

// Camera movement
camera->SetPosition(GameEngine::Vector3(0, 10, 15));
camera->LookAt(GameEngine::Vector3(0, 0, 0));
```

### Rendering Pipelines
The engine supports multiple rendering pipelines:

```cpp
// Switch rendering pipelines at runtime
engine.GetRenderManager()->SetPipeline(GameEngine::RenderPipelineType::Forward);
engine.GetRenderManager()->SetPipeline(GameEngine::RenderPipelineType::Deferred);
engine.GetRenderManager()->SetPipeline(GameEngine::RenderPipelineType::Raytracing);
```

### Texture System
```cpp
#include "Rendering/Core/Texture.h"

// Load textures with mipmap support
auto texture = std::make_shared<GameEngine::Texture>();
texture->LoadFromFile("assets/textures/brick_wall.jpg");
texture->GenerateMipmaps();
texture->SetFilter(GameEngine::TextureFilter::Linear, GameEngine::TextureFilter::Linear);

// Create texture atlases for optimization
auto atlas = std::make_shared<GameEngine::Texture>();
atlas->CreateAtlas(1024, 1024);
auto region1 = atlas->AddToAtlas("assets/textures/grass.jpg", 0, 0);
auto region2 = atlas->AddToAtlas("assets/textures/stone.jpg", 256, 0);
```

## Physics System

### 3D Physics
```cpp
// Create a dynamic rigid body
auto rigidBody = gameObject->AddComponent<GameEngine::RigidBodyComponent>();
rigidBody->SetMass(5.0f);
rigidBody->SetRestitution(0.7f); // Bounce factor
rigidBody->SetFriction(0.5f);

// Apply forces
rigidBody->AddForce(GameEngine::Vector3(0, 100, 0)); // Jump
rigidBody->AddTorque(GameEngine::Vector3(1, 0, 0)); // Spin

// Collision detection
rigidBody->OnCollisionEnter = [](GameEngine::RigidBody* other) {
    GameEngine::Logger::Info("Collision detected!");
};
```

### 2D Physics
```cpp
#include "Physics/2D/RigidBody2D.h"

// Create 2D physics objects
auto rigidBody2D = std::make_shared<GameEngine::RigidBody2D>();
rigidBody2D->SetPosition(GameEngine::Vector2(0, 5));
rigidBody2D->SetMass(2.0f);

// Add 2D colliders
auto circleCollider = std::make_shared<GameEngine::CircleCollider2D>(1.0f);
auto boxCollider = std::make_shared<GameEngine::BoxCollider2D>(GameEngine::Vector2(2, 2));
```

### Ray Casting
```cpp
#include "Core/Physics/RayCaster.h"

// 3D ray casting
GameEngine::RayCaster rayCaster;
rayCaster.Initialize(physicsWorld3D, physicsWorld2D);

GameEngine::RayHit3D hit;
if (rayCaster.Raycast3D(origin, direction, hit, 100.0f)) {
    GameEngine::Logger::Info("Hit object at distance: " + std::to_string(hit.distance));
    // hit.rigidBody contains the hit object
    // hit.point contains the hit position
    // hit.normal contains the surface normal
}

// Screen point to world ray (for mouse picking)
auto ray = rayCaster.ScreenPointToRay(mousePos, camera, screenWidth, screenHeight);
if (rayCaster.Raycast3D(ray, hit)) {
    // Handle object selection
}
```

## Audio System

### 3D Spatial Audio
```cpp
#include "Audio/AudioManager.h"
#include "Audio/AudioSource.h"

// Initialize audio system
GameEngine::AudioManager audioManager;
audioManager.Initialize();

// Load audio clips
auto footstepClip = audioManager.LoadClip("assets/audio/footstep.wav");
auto musicClip = audioManager.LoadClip("assets/audio/background_music.ogg");

// Create 3D audio source
auto audioSource = gameObject->AddComponent<GameEngine::AudioComponent>();
audioSource->SetClip(footstepClip);
audioSource->SetVolume(0.8f);
audioSource->SetPitch(1.2f);
audioSource->Set3D(true);
audioSource->SetMinDistance(1.0f);
audioSource->SetMaxDistance(50.0f);

// Play audio
audioSource->Play();
```

### Audio Effects
```cpp
// Apply audio effects
audioSource->SetReverb(true);
audioSource->SetEcho(0.3f, 0.5f);
audioSource->SetLowPassFilter(5000.0f); // Frequency cutoff
```

## Networking

### UDP Networking
```cpp
#include "Networking/UDPSocket.h"

// Create UDP server
GameEngine::UDPSocket server;
server.Bind(8080);

// Handle incoming messages
server.SetMessageHandler([](const std::string& message, const std::string& sender) {
    GameEngine::Logger::Info("Received: " + message + " from " + sender);
});

// Send messages
server.SendTo("Hello Client!", "192.168.1.100", 8081);
```

### TCP Networking
```cpp
#include "Networking/TCPSocket.h"

// Create TCP client
GameEngine::TCPSocket client;
if (client.Connect("127.0.0.1", 8080)) {
    client.Send("Hello Server!");
    
    std::string response = client.Receive();
    GameEngine::Logger::Info("Server response: " + response);
}
```

### Network Game State Synchronization
```cpp
// Synchronize game object positions over network
void SynchronizeGameObjects() {
    for (auto& gameObject : gameObjects) {
        if (gameObject->HasNetworkComponent()) {
            auto transform = gameObject->GetTransform();
            
            // Serialize position data
            std::string posData = SerializeVector3(transform.position);
            networkManager.BroadcastMessage("POS:" + std::to_string(gameObject->GetID()) + ":" + posData);
        }
    }
}
```

## Editor and Play Mode

### Editor Mode vs Play Mode
The engine supports seamless switching between edit and play modes:

```cpp
#include "Core/Editor/PlayModeManager.h"

// Switch to play mode
GameEngine::PlayModeManager playModeManager;
playModeManager.EnterPlayMode();

// Switch back to edit mode
playModeManager.ExitPlayMode();

// Check current mode
if (playModeManager.IsInPlayMode()) {
    // Game logic runs here
    UpdateGameplay();
} else {
    // Editor logic runs here
    UpdateEditor();
}
```

### Editor UI Panels
```cpp
#include "UI/EngineUI.h"

// Create custom editor panels
auto engineUI = std::make_shared<GameEngine::EngineUI>();

// Scene hierarchy panel
engineUI->ShowSceneHierarchy(scene);

// Inspector panel for selected objects
if (selectedGameObject) {
    engineUI->ShowInspector(selectedGameObject);
}

// Viewport panel
engineUI->ShowViewport(camera, renderTexture);
```

### Input Handling
```cpp
#include "Core/Platform/Input.h"

// Input is handled in a separate thread for responsiveness
GameEngine::Input& input = GameEngine::Input::GetInstance();

// Keyboard input
if (input.IsKeyPressed(GameEngine::KeyCode::W)) {
    camera->MoveForward(speed * deltaTime);
}

// Mouse input
if (input.IsMouseButtonPressed(GameEngine::MouseButton::Left)) {
    GameEngine::Vector2 mousePos = input.GetMousePosition();
    HandleMouseClick(mousePos);
}

// Cursor lock for FPS-style games
input.SetCursorLocked(true);
```

## Custom Scripts and Components

### Creating Custom Components
```cpp
// Define a custom component
class PlayerController : public GameEngine::Component {
public:
    PlayerController(GameEngine::Entity entity) : Component(entity) {}
    
    void Update(float deltaTime) override {
        HandleMovement(deltaTime);
        HandleJumping(deltaTime);
    }
    
private:
    float moveSpeed = 5.0f;
    float jumpForce = 10.0f;
    
    void HandleMovement(float deltaTime) {
        auto& input = GameEngine::Input::GetInstance();
        GameEngine::Vector3 movement(0, 0, 0);
        
        if (input.IsKeyPressed(GameEngine::KeyCode::W)) movement.z -= 1;
        if (input.IsKeyPressed(GameEngine::KeyCode::S)) movement.z += 1;
        if (input.IsKeyPressed(GameEngine::KeyCode::A)) movement.x -= 1;
        if (input.IsKeyPressed(GameEngine::KeyCode::D)) movement.x += 1;
        
        movement = movement.Normalized() * moveSpeed * deltaTime;
        
        auto transform = GetGameObject()->GetTransform();
        transform.position += movement;
        GetGameObject()->SetTransform(transform);
    }
};
```

### Attaching Custom Scripts to GameObjects
```cpp
// Add custom component to GameObject
auto player = std::make_unique<GameEngine::GameObject>(world, entity);
auto playerController = player->AddComponent<PlayerController>();

// Custom components are automatically updated by the engine
```

### MonoBehaviour-like Script System
```cpp
// Create a script that behaves like Unity's MonoBehaviour
class EnemyAI : public GameEngine::Component {
public:
    void Start() override {
        // Called when component is first created
        target = FindGameObjectWithTag("Player");
        patrolPoints = FindGameObjectsWithTag("PatrolPoint");
    }
    
    void Update(float deltaTime) override {
        // Called every frame
        switch (currentState) {
            case State::Patrol:
                UpdatePatrol(deltaTime);
                break;
            case State::Chase:
                UpdateChase(deltaTime);
                break;
        }
    }
    
    void OnTriggerEnter(GameEngine::RigidBody* other) override {
        // Called when entering a trigger volume
        if (other->GetGameObject()->HasTag("Player")) {
            currentState = State::Chase;
        }
    }
    
private:
    enum class State { Patrol, Chase };
    State currentState = State::Patrol;
    GameEngine::GameObject* target = nullptr;
    std::vector<GameEngine::GameObject*> patrolPoints;
};
```

## Advanced Features

### Animation System
```cpp
#include "Animation/Animator.h"

// Load skeletal animations
auto animator = gameObject->AddComponent<GameEngine::Animator>();
animator->LoadAnimationClip("assets/animations/walk.fbx", "walk");
animator->LoadAnimationClip("assets/animations/run.fbx", "run");
animator->LoadAnimationClip("assets/animations/jump.fbx", "jump");

// Create blend tree for smooth transitions
animator->CreateBlendTree("movement");
animator->AddBlendState("movement", "idle", 0.0f);
animator->AddBlendState("movement", "walk", 0.5f);
animator->AddBlendState("movement", "run", 1.0f);

// Control animations
float speed = GetMovementSpeed();
animator->SetBlendParameter("movement", speed);
```

### Post-Processing Effects
```cpp
#include "Rendering/PostProcessing/PostProcessingStack.h"

// Add post-processing effects
auto postProcessing = renderManager->GetPostProcessingStack();
postProcessing->AddEffect("bloom");
postProcessing->AddEffect("tone_mapping");
postProcessing->AddEffect("color_grading");

// Configure effect parameters
postProcessing->SetEffectParameter("bloom", "threshold", 1.0f);
postProcessing->SetEffectParameter("bloom", "intensity", 0.5f);
```

### Memory Management
```cpp
#include "Core/Memory/MemoryManager.h"

// Use engine's memory manager for optimal performance
GameEngine::MemoryManager& memManager = GameEngine::MemoryManager::GetInstance();

// Allocate memory pools for frequently created objects
memManager.CreatePool<GameEngine::GameObject>(1000);
memManager.CreatePool<GameEngine::MeshComponent>(500);

// Objects are automatically returned to pools when destroyed
```

## Best Practices

### Performance Optimization
1. **Use Object Pooling**: Reuse GameObjects instead of creating/destroying them frequently
2. **Batch Rendering**: Group similar objects to reduce draw calls
3. **LOD System**: Use different detail levels based on distance
4. **Frustum Culling**: Only render objects visible to the camera

```cpp
// Object pooling example
class BulletPool {
private:
    std::queue<std::unique_ptr<GameEngine::GameObject>> availableBullets;
    
public:
    GameEngine::GameObject* GetBullet() {
        if (availableBullets.empty()) {
            return CreateNewBullet();
        }
        
        auto bullet = std::move(availableBullets.front());
        availableBullets.pop();
        return bullet.release();
    }
    
    void ReturnBullet(GameEngine::GameObject* bullet) {
        bullet->SetActive(false);
        availableBullets.push(std::unique_ptr<GameEngine::GameObject>(bullet));
    }
};
```

### Code Organization
1. **Separate Logic**: Keep game logic separate from rendering code
2. **Use Systems**: Implement game features as ECS systems
3. **Component Composition**: Prefer composition over inheritance
4. **Event-Driven Architecture**: Use events for loose coupling

```cpp
// Event system example
class GameEventManager {
public:
    template<typename EventType>
    void Subscribe(std::function<void(const EventType&)> handler) {
        // Subscribe to events
    }
    
    template<typename EventType>
    void Publish(const EventType& event) {
        // Publish events to all subscribers
    }
};

// Usage
eventManager.Subscribe<PlayerDeathEvent>([](const PlayerDeathEvent& event) {
    // Handle player death
    ShowGameOverScreen();
    SaveHighScore(event.score);
});
```

### Debugging and Profiling
1. **Use Logger**: Leverage the built-in logging system
2. **Visual Debugging**: Draw debug information in the viewport
3. **Performance Profiling**: Monitor frame times and memory usage

```cpp
// Debug drawing
void DrawDebugInfo() {
    // Draw physics colliders
    for (auto& rigidBody : physicsWorld->GetRigidBodies()) {
        DrawWireframeCube(rigidBody->GetPosition(), rigidBody->GetSize());
    }
    
    // Draw light ranges
    for (auto& light : lightManager->GetLights()) {
        if (light->GetType() == GameEngine::LightType::Point) {
            DrawWireSphere(light->GetPosition(), light->GetRange());
        }
    }
}
```

### Cross-Platform Considerations
1. **File Paths**: Use forward slashes for asset paths
2. **Endianness**: Be aware of byte order when networking
3. **Input Handling**: Test on different input devices
4. **Performance**: Profile on target platforms

This tutorial covers the essential aspects of game development with GameEngineDarkest. The engine provides a solid foundation for creating both 2D and 3D games with modern rendering techniques, robust physics simulation, and comprehensive tooling support.

For more advanced topics and API reference, consult the engine documentation and example projects included in the repository.
