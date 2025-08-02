#include "Core/Scripting/External/ExternalScript.h"
#include "Core/ECS/World.h"
#include "Core/Components/TransformComponent.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Quaternion.h"
#include "Core/Logging/Logger.h"
#include <cmath>

using namespace GameEngine;

class TestCubeRotator : public IExternalScript {
private:
    float m_rotationSpeed = 1.0f;
    
public:
    void OnStart(World* world, Entity entity) override {
        Logger::Info("TestCubeRotator script started for entity: " + std::to_string(entity.GetID()));
    }
    
    void OnUpdate(World* world, Entity entity, float deltaTime) override {
        auto* transform = world->GetComponent<TransformComponent>(entity);
        if (transform) {
            Vector3 currentRotation = transform->transform.GetRotation().ToEulerAngles();
            currentRotation.y += m_rotationSpeed * deltaTime;
            transform->transform.SetRotation(Quaternion::FromEulerAngles(currentRotation.x, currentRotation.y, currentRotation.z));
        }
    }
    
    void OnDestroy(World* world, Entity entity) override {
        Logger::Info("TestCubeRotator script destroyed for entity: " + std::to_string(entity.GetID()));
    }
};

extern "C" {
#ifdef _WIN32
    __declspec(dllexport) IExternalScript* CreateScript() {
        return new TestCubeRotator();
    }
    
    __declspec(dllexport) void DestroyScript(IExternalScript* script) {
        delete script;
    }
#else
    IExternalScript* CreateScript() {
        return new TestCubeRotator();
    }
    
    void DestroyScript(IExternalScript* script) {
        delete script;
    }
#endif
}
