# Visual Studio Integration for GameEngine

This document explains how to use Visual Studio to program play mode logic for the GameEngine.

## Overview

The GameEngine supports external C++ scripting through a dynamic loading system that allows you to write game logic in Visual Studio and have it execute during play mode. Scripts are compiled to DLLs (Windows) or shared objects (Linux) and loaded at runtime.

## Quick Start

### 1. Create a Script

Create a new C++ file in the `Scripts/` directory that implements the `IExternalScript` interface:

```cpp
#include "../src/Core/Scripting/External/ExternalScript.h"
#include "../src/Core/Components/TransformComponent.h"
#include "../src/Core/Logging/Logger.h"

using namespace GameEngine;

class MyScript : public IExternalScript {
public:
    void OnStart(World* world, Entity entity) override {
        Logger::Info("Script started on entity: " + std::to_string(entity.GetID()));
    }
    
    void OnUpdate(World* world, Entity entity, float deltaTime) override {
        // Your game logic here
        auto* transform = world->GetComponent<TransformComponent>(entity);
        if (transform) {
            // Example: rotate the entity
            Vector3 rotation = transform->transform.GetRotation().ToEulerAngles();
            rotation.y += deltaTime;
            transform->transform.SetRotation(Quaternion::FromEulerAngles(rotation));
        }
    }
    
    void OnDestroy(World* world, Entity entity) override {
        Logger::Info("Script destroyed on entity: " + std::to_string(entity.GetID()));
    }
};

extern "C" {
#ifdef _WIN32
    __declspec(dllexport) IExternalScript* CreateScript() {
        return new MyScript();
    }
    
    __declspec(dllexport) void DestroyScript(IExternalScript* script) {
        delete script;
    }
#else
    IExternalScript* CreateScript() {
        return new MyScript();
    }
    
    void DestroyScript(IExternalScript* script) {
        delete script;
    }
#endif
}
```

### 2. Compile the Script

#### Windows (Visual Studio)
1. Use the provided `ScriptTemplate.vcxproj` as a starting point
2. Or run the build script: `BUILD_SCRIPT_WINDOWS.bat MyScript.cpp`

#### Linux
Run the build script: `./BUILD_SCRIPT_LINUX.sh MyScript.cpp`

### 3. Load the Script in Engine

The ExternalScriptManager automatically watches the Scripts directory for new DLLs and loads them at runtime. Scripts can be attached to entities through the editor or programmatically.

## Script API Reference

### IExternalScript Interface

All scripts must inherit from `IExternalScript` and implement these methods:

- `OnStart(World* world, Entity entity)` - Called when the script is first attached to an entity
- `OnUpdate(World* world, Entity entity, float deltaTime)` - Called every frame during play mode
- `OnDestroy(World* world, Entity entity)` - Called when the script is removed or the entity is destroyed

### Available Components

Scripts can access and modify entity components through the World pointer:

- `TransformComponent` - Position, rotation, scale
- `CameraComponent` - Camera properties
- `MovementComponent` - Movement parameters
- `RigidBodyComponent` - Physics properties
- `LightComponent` - Light properties

### Example Usage

```cpp
// Get a component
auto* transform = world->GetComponent<TransformComponent>(entity);

// Modify component data
if (transform) {
    transform->transform.SetPosition(Vector3(0, 1, 0));
}

// Log information
Logger::Info("Entity position: " + transform->transform.GetPosition().ToString());
```

## Hot Reload

The ExternalScriptManager supports hot reload - when you recompile a script, the engine will automatically reload it without restarting. This enables rapid iteration during development.

## Default Scene

The engine starts with a simplified default scene containing:
- One camera positioned at (0, 5, 10) looking at the origin
- One cube at the origin with slight rotation to demonstrate 3D nature
- One point light at (0, 3, 0) providing illumination

This provides a clean starting point for testing your scripts.

## Troubleshooting

### Common Issues

1. **Script not loading**: Ensure the DLL/SO file is in the Scripts directory and exports the required functions
2. **Compilation errors**: Check that all required headers are included and library paths are correct
3. **Runtime crashes**: Verify that you're not accessing null pointers when getting components

### Build Requirements

- Windows: Visual Studio 2019 or later with C++20 support
- Linux: GCC with C++20 support
- CMake 3.16 or later for building the main engine

## Examples

See the `Scripts/ExampleCubeRotator.cpp` file for a complete working example that rotates a cube entity.
