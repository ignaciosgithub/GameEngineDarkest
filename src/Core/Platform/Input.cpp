#include "Input.h"
#include "InputThread.h"
#include "../Logging/Logger.h"
#include <GLFW/glfw3.h>

namespace GameEngine {

InputManager::InputManager() {
    m_mousePosition = Vector3::Zero;
    m_previousMousePosition = Vector3::Zero;
    m_mouseDelta = Vector3::Zero;
}

InputManager::~InputManager() {
    Shutdown();
}

void InputManager::Initialize() {
    if (m_initialized) {
        Logger::Warning("InputManager already initialized");
        return;
    }
    
    m_inputThread = std::make_unique<InputThread>();
    m_inputThread->Initialize(this);
    m_inputThread->Start();
    
    m_initialized = true;
    Logger::Info("InputManager with threaded input initialized successfully");
}

void InputManager::Shutdown() {
    if (!m_initialized) return;
    
    if (m_inputThread) {
        m_inputThread->Stop();
        m_inputThread.reset();
    }
    
    m_initialized = false;
    Logger::Info("InputManager shutdown successfully");
}

void InputManager::Update() {
    std::lock_guard<std::mutex> lock(m_inputMutex);
    
    for (int i = 0; i < 512; ++i) {
        m_previousKeyStates[i] = m_currentKeyStates[i];
    }
    
    for (int i = 0; i < 8; ++i) {
        m_previousMouseStates[i] = m_currentMouseStates[i];
    }
    
    m_mouseDelta = m_mousePosition - m_previousMousePosition;
    m_previousMousePosition = m_mousePosition;
}

bool InputManager::IsKeyPressed(KeyCode key) const {
    std::lock_guard<std::mutex> lock(m_inputMutex);
    int keyIndex = static_cast<int>(key);
    if (keyIndex >= 0 && keyIndex < 512) {
        return m_currentKeyStates[keyIndex];
    }
    return false;
}

bool InputManager::IsKeyJustPressed(KeyCode key) const {
    std::lock_guard<std::mutex> lock(m_inputMutex);
    int keyIndex = static_cast<int>(key);
    if (keyIndex >= 0 && keyIndex < 512) {
        return m_currentKeyStates[keyIndex] && !m_previousKeyStates[keyIndex];
    }
    return false;
}

bool InputManager::IsKeyJustReleased(KeyCode key) const {
    std::lock_guard<std::mutex> lock(m_inputMutex);
    int keyIndex = static_cast<int>(key);
    if (keyIndex >= 0 && keyIndex < 512) {
        return !m_currentKeyStates[keyIndex] && m_previousKeyStates[keyIndex];
    }
    return false;
}

bool InputManager::IsMouseButtonPressed(MouseButton button) const {
    std::lock_guard<std::mutex> lock(m_inputMutex);
    int buttonIndex = static_cast<int>(button);
    if (buttonIndex >= 0 && buttonIndex < 8) {
        return m_currentMouseStates[buttonIndex];
    }
    return false;
}

bool InputManager::IsMouseButtonJustPressed(MouseButton button) const {
    std::lock_guard<std::mutex> lock(m_inputMutex);
    int buttonIndex = static_cast<int>(button);
    if (buttonIndex >= 0 && buttonIndex < 8) {
        return m_currentMouseStates[buttonIndex] && !m_previousMouseStates[buttonIndex];
    }
    return false;
}

bool InputManager::IsMouseButtonJustReleased(MouseButton button) const {
    std::lock_guard<std::mutex> lock(m_inputMutex);
    int buttonIndex = static_cast<int>(button);
    if (buttonIndex >= 0 && buttonIndex < 8) {
        return !m_currentMouseStates[buttonIndex] && m_previousMouseStates[buttonIndex];
    }
    return false;
}

Vector3 InputManager::GetMousePosition() const {
    std::lock_guard<std::mutex> lock(m_inputMutex);
    return m_mousePosition;
}

Vector3 InputManager::GetMouseDelta() const {
    std::lock_guard<std::mutex> lock(m_inputMutex);
    return m_mouseDelta;
}

Vector3 InputManager::GetMovementInput() const {
    Vector3 movement = Vector3::Zero;
    
    if (IsKeyPressed(KeyCode::W)) movement.z += 1.0f;
    if (IsKeyPressed(KeyCode::S)) movement.z -= 1.0f;
    if (IsKeyPressed(KeyCode::A)) movement.x -= 1.0f;
    if (IsKeyPressed(KeyCode::D)) movement.x += 1.0f;
    if (IsKeyPressed(KeyCode::Space)) movement.y += 1.0f;
    if (IsKeyPressed(KeyCode::LeftShift)) movement.y -= 1.0f;
    
    return movement.Normalized();
}

void InputManager::OnKeyEvent(int key, int scancode, int action, int mods) {
    if (m_inputThread && m_inputThread->IsRunning()) {
        m_inputThread->QueueKeyEvent(key, scancode, action, mods);
    } else {
        OnKeyEventThreaded(key, scancode, action, mods);
    }
}

void InputManager::OnMouseButtonEvent(int button, int action, int mods) {
    if (m_inputThread && m_inputThread->IsRunning()) {
        m_inputThread->QueueMouseButtonEvent(button, action, mods);
    } else {
        OnMouseButtonEventThreaded(button, action, mods);
    }
}

void InputManager::OnMouseMoveEvent(double xpos, double ypos) {
    if (m_inputThread && m_inputThread->IsRunning()) {
        m_inputThread->QueueMouseMoveEvent(xpos, ypos);
    } else {
        OnMouseMoveEventThreaded(xpos, ypos);
    }
}

void InputManager::OnKeyEventThreaded(int key, int /*scancode*/, int action, int /*mods*/) {
    std::lock_guard<std::mutex> lock(m_inputMutex);
    if (key >= 0 && key < 512) {
        m_currentKeyStates[key] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
}

void InputManager::OnMouseButtonEventThreaded(int button, int action, int /*mods*/) {
    std::lock_guard<std::mutex> lock(m_inputMutex);
    if (button >= 0 && button < 8) {
        m_currentMouseStates[button] = (action == GLFW_PRESS);
    }
}

void InputManager::OnMouseMoveEventThreaded(double xpos, double ypos) {
    std::lock_guard<std::mutex> lock(m_inputMutex);
    m_mousePosition.x = static_cast<float>(xpos);
    m_mousePosition.y = static_cast<float>(ypos);
}

}
