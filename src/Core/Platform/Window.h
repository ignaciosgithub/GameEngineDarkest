#pragma once

#include <string>
#include <functional>

struct GLFWwindow;

namespace GameEngine {
    class Window {
    public:
        using ResizeCallback = std::function<void(int width, int height)>;
        using KeyCallback = std::function<void(int key, int scancode, int action, int mods)>;
        using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
        using MouseMoveCallback = std::function<void(double xpos, double ypos)>;
        
        Window();
        ~Window();
        
        bool Create(const std::string& title, int width, int height);
        void Destroy();
        
        bool ShouldClose() const;
        void SwapBuffers();
        void PollEvents();
        
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        float GetAspectRatio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
        
        GLFWwindow* GetGLFWWindow() const { return m_window; }
        
        // Event callbacks
        void SetResizeCallback(const ResizeCallback& callback) { m_resizeCallback = callback; }
        void SetKeyCallback(const KeyCallback& callback) { m_keyCallback = callback; }
        void SetMouseButtonCallback(const MouseButtonCallback& callback) { m_mouseButtonCallback = callback; }
        void SetMouseMoveCallback(const MouseMoveCallback& callback) { m_mouseMoveCallback = callback; }
        
        // Input state
        bool IsKeyPressed(int key) const;
        bool IsMouseButtonPressed(int button) const;
        void GetMousePosition(double& x, double& y) const;
        
    private:
        static void OnWindowResize(GLFWwindow* window, int width, int height);
        static void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void OnMouseButton(GLFWwindow* window, int button, int action, int mods);
        static void OnMouseMove(GLFWwindow* window, double xpos, double ypos);
        
        GLFWwindow* m_window = nullptr;
        int m_width = 0;
        int m_height = 0;
        
        ResizeCallback m_resizeCallback;
        KeyCallback m_keyCallback;
        MouseButtonCallback m_mouseButtonCallback;
        MouseMoveCallback m_mouseMoveCallback;
    };
}
