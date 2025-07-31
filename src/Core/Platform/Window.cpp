#include "../../Rendering/Core/OpenGLHeaders.h"
#include "Window.h"
#include "../Logging/Logger.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace GameEngine {

Window::Window() = default;

Window::~Window() {
    Destroy();
}

bool Window::Create(const std::string& title, int width, int height) {
    m_width = width;
    m_height = height;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);  // Create invisible window for headless rendering
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        Logger::Error("Failed to create GLFW window");
        return false;
    }
    
    glfwSetWindowUserPointer(m_window, this);
    
    glfwSetWindowSizeCallback(m_window, OnWindowResize);
    glfwSetKeyCallback(m_window, OnKey);
    glfwSetMouseButtonCallback(m_window, OnMouseButton);
    glfwSetCursorPosCallback(m_window, OnMouseMove);
    
    glfwMakeContextCurrent(m_window);
    
#ifdef _WIN32
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Logger::Error("Failed to initialize GLAD");
        return false;
    }
    Logger::Info("GLAD initialized successfully");
#else
    if (glewInit() != GLEW_OK) {
        Logger::Error("Failed to initialize GLEW");
        return false;
    }
    Logger::Info("GLEW initialized successfully");
#endif
    
    glfwSwapInterval(1);
    
    Logger::Info("Window created: " + title + " (" + std::to_string(width) + "x" + std::to_string(height) + ")");
    return true;
}

void Window::Destroy() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        Logger::Info("Window destroyed");
    }
}

bool Window::ShouldClose() const {
    return m_window && glfwWindowShouldClose(m_window);
}

void Window::SwapBuffers() {
    if (m_window) {
        glfwSwapBuffers(m_window);
    }
}

void Window::PollEvents() {
    glfwPollEvents();
}

bool Window::IsKeyPressed(int key) const {
    if (!m_window) return false;
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Window::IsMouseButtonPressed(int button) const {
    if (!m_window) return false;
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void Window::GetMousePosition(double& x, double& y) const {
    if (m_window) {
        glfwGetCursorPos(m_window, &x, &y);
    } else {
        x = y = 0.0;
    }
}

void Window::OnWindowResize(GLFWwindow* window, int width, int height) {
    Window* windowInstance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (windowInstance) {
        windowInstance->m_width = width;
        windowInstance->m_height = height;
        
        glViewport(0, 0, width, height);
        
        if (windowInstance->m_resizeCallback) {
            windowInstance->m_resizeCallback(width, height);
        }
    }
}

void Window::OnKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* windowInstance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (windowInstance && windowInstance->m_keyCallback) {
        windowInstance->m_keyCallback(key, scancode, action, mods);
    }
}

void Window::OnMouseButton(GLFWwindow* window, int button, int action, int mods) {
    Window* windowInstance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (windowInstance && windowInstance->m_mouseButtonCallback) {
        windowInstance->m_mouseButtonCallback(button, action, mods);
    }
}

void Window::OnMouseMove(GLFWwindow* window, double xpos, double ypos) {
    Window* windowInstance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (windowInstance && windowInstance->m_mouseMoveCallback) {
        windowInstance->m_mouseMoveCallback(xpos, ypos);
    }
}

}
