#pragma once

#include "../../ECS/World.h"
#include "../../ECS/Entity.h"
#include <functional>
#include <string>

namespace GameEngine {
    
    class IExternalScript {
    public:
        virtual ~IExternalScript() = default;
        virtual void OnStart(World* world, Entity entity) = 0;
        virtual void OnUpdate(World* world, Entity entity, float deltaTime) = 0;
        virtual void OnDestroy(World* world, Entity entity) = 0;
    };
    
    class ExternalScript {
    public:
        ExternalScript(const std::string& name, void* libraryHandle);
        ~ExternalScript();
        
        bool Initialize();
        void ExecuteStart(World* world, Entity entity);
        void ExecuteUpdate(World* world, Entity entity, float deltaTime);
        void ExecuteDestroy(World* world, Entity entity);
        
        const std::string& GetName() const { return m_name; }
        bool IsValid() const { return m_scriptInstance != nullptr; }
        
    private:
        std::string m_name;
        void* m_libraryHandle;
        IExternalScript* m_scriptInstance;
        
        typedef IExternalScript* (*CreateScriptFunc)();
        typedef void (*DestroyScriptFunc)(IExternalScript*);
        
        CreateScriptFunc m_createFunc;
        DestroyScriptFunc m_destroyFunc;
    };
}
