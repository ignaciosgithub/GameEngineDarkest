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
    Memory/MemoryManager.cpp
    Logging/Logger.cpp
    Components/RigidBodyComponent.cpp
    Components/MeshComponent.cpp
    Systems/CameraSystem.cpp
    Systems/MovementSystem.cpp
    Scenes/TestSceneManager.cpp
    Scenes/Scene.cpp
    GameObject/GameObject.cpp
    GameObject/Prefab.cpp
    Engine.cpp
)

target_include_directories(Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${GLFW3_INCLUDE_DIRS}
)

target_link_libraries(Core PUBLIC
    ${GLFW_LIBRARIES}
    ${GLM_LIBRARIES}
    OpenGL::GL
)

target_compile_features(Core PUBLIC cxx_std_20)
