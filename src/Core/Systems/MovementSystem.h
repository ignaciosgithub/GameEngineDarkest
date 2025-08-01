#pragma once

#include "../ECS/System.h"

namespace GameEngine {
    class InputManager;
    class Window;
    class PlayModeManager;
    enum class EditorMode;
    
    class MovementSystem : public System<MovementSystem> {
    public:
        MovementSystem(InputManager* inputManager, Window* window, PlayModeManager* playModeManager);
        
        void OnUpdate(World* world, float deltaTime) override;
        void OnInitialize(World* world) override;
        
    private:
        void UpdateMovement(World* world, float deltaTime);
        void UpdateMouseLook(World* world, float deltaTime);
        
        InputManager* m_inputManager;
        Window* m_window;
        PlayModeManager* m_playModeManager;
    };
}
