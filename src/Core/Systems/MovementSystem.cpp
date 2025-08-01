#include "MovementSystem.h"
#include "../ECS/World.h"
#include "../Components/MovementComponent.h"
#include "../Components/TransformComponent.h"
#include "../Platform/Input.h"
#include "../Platform/Window.h"
#include "../Logging/Logger.h"
#include <GLFW/glfw3.h>
#include <cmath>

namespace GameEngine {

MovementSystem::MovementSystem(InputManager* inputManager, Window* window)
    : m_inputManager(inputManager), m_window(window) {
}

void MovementSystem::OnInitialize(World* /*world*/) {
    Logger::Info("MovementSystem initialized");
}

void MovementSystem::OnUpdate(World* world, float deltaTime) {
    UpdateMovement(world, deltaTime);
    UpdateMouseLook(world, deltaTime);
}

void MovementSystem::UpdateMovement(World* world, float deltaTime) {
    if (!m_inputManager) return;
    
    Vector3 movementInput = m_inputManager->GetMovementInput();
    
    for (const auto& entity : world->GetEntities()) {
        auto* movement = world->GetComponent<MovementComponent>(entity);
        auto* transform = world->GetComponent<TransformComponent>(entity);
        
        if (movement && transform) {
            Vector3 forward = transform->transform.GetForward();
            Vector3 right = transform->transform.GetRight();
            Vector3 up = Vector3::Up; // World up for vertical movement
            
            Vector3 moveDirection = Vector3::Zero;
            moveDirection += right * movementInput.x;      // A/D
            moveDirection += up * movementInput.y;         // Space/Shift
            moveDirection += forward * movementInput.z;    // W/S
            
            if (moveDirection.LengthSquared() > 0.0f) {
                moveDirection.Normalize();
                movement->velocity = moveDirection * movement->movementSpeed;
            } else {
                movement->velocity = Vector3::Zero;
            }
            
            Vector3 displacement = movement->velocity * deltaTime;
            transform->transform.Translate(displacement);
        }
    }
}

void MovementSystem::UpdateMouseLook(World* world, float /*deltaTime*/) {
    if (!m_inputManager || !m_window) return;
    
    if (!m_inputManager->IsMouseButtonPressed(MouseButton::Right)) {
        return;
    }
    
    Vector3 mousePos = m_inputManager->GetMousePosition();
    
    for (const auto& entity : world->GetEntities()) {
        auto* movement = world->GetComponent<MovementComponent>(entity);
        auto* transform = world->GetComponent<TransformComponent>(entity);
        
        if (movement && transform) {
            if (movement->firstMouse) {
                movement->lastMousePos = mousePos;
                movement->firstMouse = false;
            }
            
            Vector3 mouseDelta = mousePos - movement->lastMousePos;
            movement->lastMousePos = mousePos;
            
            mouseDelta *= movement->mouseSensitivity * 0.01f;
            
            movement->yaw += mouseDelta.x;
            movement->pitch -= mouseDelta.y; // Invert Y axis
            
            const float maxPitch = 89.0f * 3.14159f / 180.0f; // Convert to radians
            if (movement->pitch > maxPitch) movement->pitch = maxPitch;
            if (movement->pitch < -maxPitch) movement->pitch = -maxPitch;
            
            Quaternion yawRotation = Quaternion::FromAxisAngle(Vector3::Up, movement->yaw);
            Quaternion pitchRotation = Quaternion::FromAxisAngle(Vector3::Right, movement->pitch);
            Quaternion finalRotation = yawRotation * pitchRotation;
            
            transform->transform.SetRotation(finalRotation);
        }
    }
}

}
