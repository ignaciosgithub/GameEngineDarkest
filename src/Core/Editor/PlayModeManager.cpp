#include "PlayModeManager.h"
#include "../ECS/World.h"
#include "../Platform/Window.h"
#include "../Logging/Logger.h"
#include "../Time/Timer.h"
#include "../Components/TransformComponent.h"
#include "../Components/MovementComponent.h"
#include <GLFW/glfw3.h>

namespace GameEngine {
    
    PlayModeManager::PlayModeManager() = default;
    
    PlayModeManager::~PlayModeManager() = default;
    
    void PlayModeManager::Initialize(World* world, Window* window) {
        if (m_initialized) {
            Logger::Warning("PlayModeManager already initialized");
            return;
        }
        
        if (!world || !window) {
            Logger::Error("PlayModeManager requires valid World and Window pointers");
            return;
        }
        
        m_world = world;
        m_window = window;
        m_initialized = true;
        
        Logger::Info("PlayModeManager initialized successfully");
    }
    
    void PlayModeManager::Update(float deltaTime) {
        if (!m_initialized) return;
        
        (void)deltaTime;
        
        switch (m_currentMode) {
            case EditorMode::Edit:
                break;
                
            case EditorMode::Play:
                break;
                
            case EditorMode::Paused:
                break;
        }
    }
    
    void PlayModeManager::SwitchToEditMode() {
        if (!m_initialized) {
            Logger::Error("PlayModeManager not initialized");
            return;
        }
        
        if (m_currentMode == EditorMode::Edit) {
            Logger::Debug("Already in Edit mode");
            return;
        }
        
        Logger::Info("Switching to Edit mode");
        
        if (m_currentMode == EditorMode::Play || m_currentMode == EditorMode::Paused) {
            RestoreSceneState();
        }
        
        if (m_cursorLocked) {
            SetCursorMode(false);
            m_cursorLocked = false;
        }
        
        Timer::Reset();
        
        m_previousMode = m_currentMode;
        m_currentMode = EditorMode::Edit;
        
        Logger::Info("Switched to Edit mode successfully");
    }
    
    void PlayModeManager::SwitchToPlayMode() {
        if (!m_initialized) {
            Logger::Error("PlayModeManager not initialized");
            return;
        }
        
        if (m_currentMode == EditorMode::Play) {
            Logger::Debug("Already in Play mode");
            return;
        }
        
        Logger::Info("Switching to Play mode");
        
        if (m_currentMode == EditorMode::Edit) {
            SaveSceneState();
        }
        
        Timer::Reset();
        
        m_previousMode = m_currentMode;
        m_currentMode = EditorMode::Play;
        
        Logger::Info("Switched to Play mode successfully");
    }
    
    void PlayModeManager::TogglePause() {
        if (!m_initialized) {
            Logger::Error("PlayModeManager not initialized");
            return;
        }
        
        if (m_currentMode == EditorMode::Edit) {
            Logger::Warning("Cannot pause in Edit mode");
            return;
        }
        
        if (m_currentMode == EditorMode::Play) {
            Logger::Info("Pausing game");
            m_previousMode = m_currentMode;
            m_currentMode = EditorMode::Paused;
            
            Timer::SetTimeScale(0.0f);
        } else if (m_currentMode == EditorMode::Paused) {
            Logger::Info("Resuming game");
            m_currentMode = m_previousMode;
            
            Timer::SetTimeScale(1.0f);
            Timer::Reset(); // Prevent large delta time spike
        }
    }
    
    void PlayModeManager::ToggleCursorLock() {
        if (!m_initialized) {
            Logger::Error("PlayModeManager not initialized");
            return;
        }
        
        m_cursorLocked = !m_cursorLocked;
        SetCursorMode(m_cursorLocked);
        
        Logger::Info("Cursor lock " + std::string(m_cursorLocked ? "enabled" : "disabled"));
    }
    
    void PlayModeManager::SaveSceneState() {
        if (!m_world) {
            Logger::Error("Cannot save scene state - World is null");
            return;
        }
        
        m_savedSceneState.isValid = true;
        m_savedSceneState.entities.clear();
        
        const auto& entities = m_world->GetEntities();
        for (const auto& entity : entities) {
            EntityState entityState;
            entityState.entityId = entity.GetID();
            
            auto* transform = m_world->GetComponent<TransformComponent>(entity);
            if (transform) {
                entityState.hasTransform = true;
                entityState.position = transform->transform.GetPosition();
                entityState.rotation = transform->transform.GetRotation();
                entityState.scale = transform->transform.GetScale();
            }
            
            auto* movement = m_world->GetComponent<MovementComponent>(entity);
            if (movement) {
                entityState.hasMovement = true;
                entityState.movementSpeed = movement->movementSpeed;
                entityState.mouseSensitivity = movement->mouseSensitivity;
                entityState.velocity = movement->velocity;
                entityState.pitch = movement->pitch;
                entityState.yaw = movement->yaw;
            }
            
            m_savedSceneState.entities.push_back(entityState);
        }
        
        Logger::Debug("Scene state saved with " + std::to_string(m_savedSceneState.entities.size()) + " entities");
    }
    
    void PlayModeManager::RestoreSceneState() {
        if (!m_world) {
            Logger::Error("Cannot restore scene state - World is null");
            return;
        }
        
        if (!m_savedSceneState.isValid) {
            Logger::Warning("No valid scene state to restore");
            return;
        }
        
        for (const auto& entityState : m_savedSceneState.entities) {
            Entity entity = Entity(entityState.entityId);
            
            if (m_world->IsEntityValid(entity)) {
                if (entityState.hasTransform) {
                    auto* transform = m_world->GetComponent<TransformComponent>(entity);
                    if (transform) {
                        transform->transform.SetPosition(entityState.position);
                        transform->transform.SetRotation(entityState.rotation);
                        transform->transform.SetScale(entityState.scale);
                    }
                }
                
                if (entityState.hasMovement) {
                    auto* movement = m_world->GetComponent<MovementComponent>(entity);
                    if (movement) {
                        movement->movementSpeed = entityState.movementSpeed;
                        movement->mouseSensitivity = entityState.mouseSensitivity;
                        movement->velocity = entityState.velocity;
                        movement->pitch = entityState.pitch;
                        movement->yaw = entityState.yaw;
                    }
                }
            }
        }
        
        Timer::Reset();
        
        Logger::Debug("Scene state restored with " + std::to_string(m_savedSceneState.entities.size()) + " entities");
    }
    
    void PlayModeManager::SetCursorMode(bool locked) {
        if (!m_window) {
            Logger::Error("Cannot set cursor mode - Window is null");
            return;
        }
        
        GLFWwindow* glfwWindow = m_window->GetGLFWWindow();
        if (!glfwWindow) {
            Logger::Error("Cannot set cursor mode - GLFW window is null");
            return;
        }
        
        if (locked) {
            glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}
