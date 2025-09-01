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

# Cross-platform package management
find_package(OpenGL REQUIRED)

# Use vcpkg on Windows, system packages on Linux
if(WIN32)
    # Windows: Use vcpkg packages
    find_package(glfw3 CONFIG REQUIRED)
    find_package(glad CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)
    find_package(imgui CONFIG REQUIRED)
    find_package(GTest CONFIG REQUIRED)
    find_package(OpenAL CONFIG)
    find_package(assimp CONFIG)
    find_package(nfd CONFIG QUIET)
    
    # Set variables for compatibility
    set(GLFW_LIBRARIES glfw)
    set(GLEW_LIBRARIES glad::glad)
    set(GLM_LIBRARIES glm::glm)
    set(GTEST_LIBRARIES GTest::gtest GTest::gtest_main)
    set(OPENAL_LIBRARIES OpenAL::OpenAL)
    set(ASSIMP_LIBRARIES assimp::assimp)
    if(nfd_FOUND)
        set(NFD_LIBRARIES nfd::nfd)
    else()
        set(NFD_LIBRARIES "")
        message(STATUS "nativefiledialog-extended not found on Windows, file browser functionality will be disabled")
    endif()
    
    # Create ImGui alias for Windows compatibility
    if(TARGET imgui::imgui)
        add_library(imgui ALIAS imgui::imgui)
    endif()
else()
    # Linux: Use system packages
    find_package(PkgConfig REQUIRED)
    find_package(glfw3 REQUIRED)
    pkg_check_modules(GLEW REQUIRED glew)
    find_package(glm REQUIRED)
    find_package(GTest REQUIRED)
    pkg_check_modules(OPENAL openal)
    find_package(assimp)
    find_package(nfd CONFIG QUIET)
    
    # ImGui for Linux - use as submodule or system package
    find_path(IMGUI_INCLUDE_DIR imgui.h PATHS /usr/include/imgui /usr/local/include/imgui)
    if(NOT IMGUI_INCLUDE_DIR)
        message(STATUS "ImGui not found in system, using fallback")
        set(IMGUI_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui")
    endif()
    
    # Set variables for compatibility
    set(GLFW_LIBRARIES glfw)
    set(GLEW_LIBRARIES ${GLEW_LIBRARIES})
    set(GLM_LIBRARIES glm::glm)
    set(GTEST_LIBRARIES ${GTEST_LIBRARIES})
    set(OPENAL_LIBRARIES ${OPENAL_LIBRARIES})
    set(ASSIMP_LIBRARIES assimp)
    if(nfd_FOUND)
        set(NFD_LIBRARIES nfd::nfd)
    else()
        set(NFD_LIBRARIES "")
        message(STATUS "nativefiledialog-extended not found, file browser functionality will be disabled")
    endif()
    
    # Create ImGui target for Linux - link system libraries
    add_library(imgui INTERFACE)
    target_include_directories(imgui INTERFACE ${IMGUI_INCLUDE_DIR})
    target_link_libraries(imgui INTERFACE -limgui -lstb)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Engine modules
add_subdirectory(src/Audio)
add_subdirectory(src/Animation)
add_subdirectory(src/Core)
add_subdirectory(src/Rendering)
add_subdirectory(src/Physics)
add_subdirectory(src/Networking)
add_subdirectory(src/UI)

# Demo application
add_subdirectory(demo)

# Editor application
add_subdirectory(editor)

# Create main engine library
add_library(GameEngine INTERFACE)
target_link_libraries(GameEngine INTERFACE
    Audio
    Animation
    Core
    Rendering
    Physics
    Networking
    UI
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
add_subdirectory(tests/physics_headless)
