# External Scripts for GameEngineDarkest

This directory contains C++ scripts that can be compiled and loaded at runtime to add custom behavior to game entities.

## How to Create a Script

1. Copy `ScriptTemplate.cpp` and rename it to your desired script name
2. Modify the class name and implement the three required methods:
   - `OnStart(World* world, Entity entity)` - Called when the script is first attached
   - `OnUpdate(World* world, Entity entity, float deltaTime)` - Called every frame
   - `OnDestroy(World* world, Entity entity)` - Called when the script is removed

3. Keep the export functions at the bottom unchanged (they're required for dynamic loading)

## Available Engine Components

You can access these components through the World* parameter:

- `TransformComponent` - Position, rotation, scale
- `CameraComponent` - Camera properties
- `MovementComponent` - Movement behavior
- `RigidBodyComponent` - Physics properties
- `LightComponent` - Lighting properties

Example usage:
```cpp
auto* transform = world->GetComponent<TransformComponent>(entity);
if (transform) {
    Vector3 position = transform->transform.GetPosition();
    transform->transform.SetPosition(Vector3(0, 1, 0));
}
```

## Compilation

Scripts are automatically compiled when you use the ExternalScriptManager:

```cpp
ExternalScriptManager::Instance().CompileScript("Scripts/MyScript.cpp");
ExternalScriptManager::Instance().LoadCompiledScript("MyScript");
ExternalScriptManager::Instance().AttachScriptToEntity(entity, "MyScript");
```

## Visual Studio Integration

To use Visual Studio for development:

1. Create a new C++ project or use an existing one
2. Add the GameEngineDarkest/src directory to your include paths
3. Write your script using IntelliSense and debugging features
4. Save the script to the Scripts/ directory
5. The engine will automatically detect changes and recompile

## Examples

- `ExampleCubeRotator.cpp` - Rotates an entity around the Y axis
- `ScriptTemplate.cpp` - Basic template showing all required functions

## Notes

- Scripts must implement the `IExternalScript` interface
- Export functions (`CreateScript` and `DestroyScript`) are required
- Use `Logger::Info()` for debugging output
- Scripts have full access to the engine's ECS system through the World pointer
