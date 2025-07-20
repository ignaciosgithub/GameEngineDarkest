cmake_minimum_required(VERSION 3.20)

project(GameEngineDarkest 
    VERSION 1.0.0
    DESCRIPTION "Modular 3D/2D Game Engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing
enable_testing()

# Find packages using vcpkg/CMake
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Engine modules
add_subdirectory(src/Core)
add_subdirectory(src/Rendering)
add_subdirectory(src/Physics)
# add_subdirectory(src/UI)  # Temporarily disabled - missing imgui headers

# Demo application
add_subdirectory(demo)

# Create main engine library
add_library(GameEngine INTERFACE)
target_link_libraries(GameEngine INTERFACE
    Core
    Rendering
    Physics
    # UI  # Temporarily disabled
)

# Export targets (commented out for demo build)
# install(TARGETS GameEngine
#     EXPORT GameEngineTargets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
# )
# 
# install(EXPORT GameEngineTargets
#     FILE GameEngineTargets.cmake
#     NAMESPACE GameEngine::
#     DESTINATION lib/cmake/GameEngine
# )
