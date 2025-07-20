#include "Input.h"
#include <GLFW/glfw3.h>

namespace GameEngine {

InputManager::InputManager() {
    m_mousePosition = Vector3::Zero;
    m_previousMousePosition = Vector3::Zero;
    m_mouseDelta = Vector3::Zero;
}

InputManager::~InputManager() = default;

void InputManager::Update() {
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
    int keyIndex = static_cast<int>(key);
    if (keyIndex >= 0 && keyIndex < 512) {
        return m_currentKeyStates[keyIndex];
    }
    return false;
}

bool InputManager::IsKeyJustPressed(KeyCode key) const {
    int keyIndex = static_cast<int>(key);
    if (keyIndex >= 0 && keyIndex < 512) {
        return m_currentKeyStates[keyIndex] && !m_previousKeyStates[keyIndex];
    }
    return false;
}

bool InputManager::IsKeyJustReleased(KeyCode key) const {
    int keyIndex = static_cast<int>(key);
    if (keyIndex >= 0 && keyIndex < 512) {
        return !m_currentKeyStates[keyIndex] && m_previousKeyStates[keyIndex];
    }
    return false;
}

bool InputManager::IsMouseButtonPressed(MouseButton button) const {
    int buttonIndex = static_cast<int>(button);
    if (buttonIndex >= 0 && buttonIndex < 8) {
        return m_currentMouseStates[buttonIndex];
    }
    return false;
}

bool InputManager::IsMouseButtonJustPressed(MouseButton button) const {
    int buttonIndex = static_cast<int>(button);
    if (buttonIndex >= 0 && buttonIndex < 8) {
        return m_currentMouseStates[buttonIndex] && !m_previousMouseStates[buttonIndex];
    }
    return false;
}

bool InputManager::IsMouseButtonJustReleased(MouseButton button) const {
    int buttonIndex = static_cast<int>(button);
    if (buttonIndex >= 0 && buttonIndex < 8) {
        return !m_currentMouseStates[buttonIndex] && m_previousMouseStates[buttonIndex];
    }
    return false;
}

Vector3 InputManager::GetMousePosition() const {
    return m_mousePosition;
}

Vector3 InputManager::GetMouseDelta() const {
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

void InputManager::OnKeyEvent(int key, int /*scancode*/, int action, int /*mods*/) {
    if (key >= 0 && key < 512) {
        m_currentKeyStates[key] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
}

void InputManager::OnMouseButtonEvent(int button, int action, int /*mods*/) {
    if (button >= 0 && button < 8) {
        m_currentMouseStates[button] = (action == GLFW_PRESS);
    }
}

void InputManager::OnMouseMoveEvent(double xpos, double ypos) {
    m_mousePosition.x = static_cast<float>(xpos);
    m_mousePosition.y = static_cast<float>(ypos);
}

}
