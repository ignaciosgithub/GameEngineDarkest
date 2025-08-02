#include "../src/Core/Scripting/External/ExternalScript.h"
#include "../src/Core/Components/TransformComponent.h"
#include "../src/Core/Logging/Logger.h"
#include "../src/Core/Math/Vector3.h"
#include "../src/Core/Math/Quaternion.h"

using namespace GameEngine;

class CubeRotator : public IExternalScript {
private:
    float m_rotationSpeed = 2.0f;
    Vector3 m_rotationAxis = Vector3(0.0f, 1.0f, 0.0f);
    
public:
    void OnStart(World* world, Entity entity) override {
        Logger::Info("CubeRotator script started on entity: " + std::to_string(entity.GetID()));
        
        auto* transform = world->GetComponent<TransformComponent>(entity);
        if (transform) {
            Logger::Info("Initial cube position: " + transform->transform.GetPosition().ToString());
        }
    }
    
    void OnUpdate(World* world, Entity entity, float deltaTime) override {
        auto* transform = world->GetComponent<TransformComponent>(entity);
        if (transform) {
            Vector3 currentRotation = transform->transform.GetRotation().ToEulerAngles();
            currentRotation.y += deltaTime * m_rotationSpeed;
            transform->transform.SetRotation(Quaternion::FromEulerAngles(currentRotation));
        }
    }
    
    void OnDestroy(World* world, Entity entity) override {
        Logger::Info("CubeRotator script destroyed on entity: " + std::to_string(entity.GetID()));
    }
};

extern "C" {
#ifdef _WIN32
    __declspec(dllexport) IExternalScript* CreateScript() {
        return new CubeRotator();
    }
    
    __declspec(dllexport) void DestroyScript(IExternalScript* script) {
        delete script;
    }
#else
    IExternalScript* CreateScript() {
        return new CubeRotator();
    }
    
    void DestroyScript(IExternalScript* script) {
        delete script;
    }
#endif
}
