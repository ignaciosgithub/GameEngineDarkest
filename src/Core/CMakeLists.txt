add_library(Core STATIC
    ECS/Entity.cpp
    ECS/Component.cpp
    ECS/System.cpp
    ECS/World.cpp
    Math/Vector2.cpp
    Math/Vector3.cpp
    Math/Vector4.cpp
    Math/Matrix4.cpp
    Math/Quaternion.cpp
    Math/Transform.cpp
    Platform/Window.cpp
    Platform/Input.cpp
    Platform/InputThread.cpp
    Memory/MemoryManager.cpp
    Logging/Logger.cpp
    Components/RigidBodyComponent.cpp
    Components/MeshComponent.cpp
    Components/AudioComponent.cpp
    Systems/CameraSystem.cpp
    Systems/MovementSystem.cpp
    Scenes/TestSceneManager.cpp
    Scenes/Scene.cpp
    GameObject/GameObject.cpp
    GameObject/Prefab.cpp
    Time/Timer.cpp
    Editor/PlayModeManager.cpp
    Physics/RayCaster.cpp
    Engine.cpp
)

target_include_directories(Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${GLFW3_INCLUDE_DIRS}
)

target_link_libraries(Core PUBLIC
    UI
    Audio
    ${GLFW_LIBRARIES}
    ${GLM_LIBRARIES}
    OpenGL::GL
)

target_compile_features(Core PUBLIC cxx_std_20)

# Test executables
add_executable(LightIntegrationTest Tests/LightIntegrationTest.cpp)
target_link_libraries(LightIntegrationTest Core Rendering)
