#pragma once

#include <memory>

namespace GameEngine {
    class World;
    class Window;
    
    enum class EditorMode {
        Edit,
        Play,
        Paused
    };
    
    class PlayModeManager {
    public:
        PlayModeManager();
        ~PlayModeManager();
        
        void Initialize(World* world, Window* window);
        void Update(float deltaTime);
        
        void SwitchToEditMode();
        void SwitchToPlayMode();
        void TogglePause();
        void ToggleCursorLock();
        
        EditorMode GetCurrentMode() const { return m_currentMode; }
        bool IsCursorLocked() const { return m_cursorLocked; }
        bool IsInPlayMode() const { return m_currentMode == EditorMode::Play; }
        bool IsInEditMode() const { return m_currentMode == EditorMode::Edit; }
        bool IsPaused() const { return m_currentMode == EditorMode::Paused; }
        
    private:
        void SaveSceneState();
        void RestoreSceneState();
        void SetCursorMode(bool locked);
        
        EditorMode m_currentMode = EditorMode::Edit;
        EditorMode m_previousMode = EditorMode::Edit;
        bool m_cursorLocked = false;
        
        World* m_world = nullptr;
        Window* m_window = nullptr;
        
        struct SceneState {
            // Store entity states for restoration
            bool isValid = false;
        } m_savedSceneState;
        
        bool m_initialized = false;
    };
}
