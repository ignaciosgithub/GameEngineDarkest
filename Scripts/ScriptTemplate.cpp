#include "../src/Core/Scripting/External/ExternalScript.h"
#include "../src/Core/Components/TransformComponent.h"
#include "../src/Core/Logging/Logger.h"

using namespace GameEngine;

class MyScript : public IExternalScript {
public:
    void OnStart(World* world, Entity entity) override {
        Logger::Info("MyScript OnStart called for entity: " + std::to_string(entity.GetID()));
        
        auto* transform = world->GetComponent<TransformComponent>(entity);
        if (transform) {
            Logger::Info("Entity position: " + transform->transform.GetPosition().ToString());
        }
    }
    
    void OnUpdate(World* world, Entity entity, float deltaTime) override {
        auto* transform = world->GetComponent<TransformComponent>(entity);
        if (transform) {
            Vector3 currentRotation = transform->transform.GetRotation().ToEulerAngles();
            currentRotation.y += deltaTime * 1.0f; // Rotate 1 radian per second around Y axis
            transform->transform.SetRotation(Quaternion::FromEulerAngles(currentRotation));
        }
    }
    
    void OnDestroy(World* world, Entity entity) override {
        Logger::Info("MyScript OnDestroy called for entity: " + std::to_string(entity.GetID()));
    }
};

extern "C" {
    __declspec(dllexport) IExternalScript* CreateScript() {
        return new MyScript();
    }
    
    __declspec(dllexport) void DestroyScript(IExternalScript* script) {
        delete script;
    }
}
