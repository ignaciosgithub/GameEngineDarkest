#pragma once

#include "../Math/Vector3.h"

namespace GameEngine {
    enum class KeyCode {
        Unknown = -1,
        Space = 32,
        Key0 = 48, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9,
        A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        Escape = 256,
        Enter = 257,
        Tab = 258,
        Backspace = 259,
        Insert = 260,
        Delete = 261,
        Right = 262,
        Left = 263,
        Down = 264,
        Up = 265,
        LeftShift = 340,
        LeftControl = 341,
        LeftAlt = 342,
        LeftSuper = 343,
        RightShift = 344,
        RightControl = 345,
        RightAlt = 346,
        RightSuper = 347
    };
    
    enum class MouseButton {
        Left = 0,
        Right = 1,
        Middle = 2
    };
    
    class InputManager {
    public:
        InputManager();
        ~InputManager();
        
        void Update();
        
        // Keyboard input
        bool IsKeyPressed(KeyCode key) const;
        bool IsKeyJustPressed(KeyCode key) const;
        bool IsKeyJustReleased(KeyCode key) const;
        
        // Mouse input
        bool IsMouseButtonPressed(MouseButton button) const;
        bool IsMouseButtonJustPressed(MouseButton button) const;
        bool IsMouseButtonJustReleased(MouseButton button) const;
        
        Vector3 GetMousePosition() const;
        Vector3 GetMouseDelta() const;
        
        // WASD movement helper
        Vector3 GetMovementInput() const;
        
    private:
        void OnKeyEvent(int key, int scancode, int action, int mods);
        void OnMouseButtonEvent(int button, int action, int mods);
        void OnMouseMoveEvent(double xpos, double ypos);
        
        friend class Window;
        
        bool m_currentKeyStates[512] = {};
        bool m_previousKeyStates[512] = {};
        
        bool m_currentMouseStates[8] = {};
        bool m_previousMouseStates[8] = {};
        
        Vector3 m_mousePosition;
        Vector3 m_previousMousePosition;
        Vector3 m_mouseDelta;
    };
}
