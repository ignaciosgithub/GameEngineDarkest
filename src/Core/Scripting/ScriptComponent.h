#pragma once

#include "../ECS/Component.h"
#include <string>
#include <functional>

namespace GameEngine {
    class World;
    class Entity;
    
    class ScriptComponent : public Component<ScriptComponent> {
    public:
        ScriptComponent() = default;
        ScriptComponent(const std::string& scriptPath) : m_scriptPath(scriptPath) {}
        
        std::string m_scriptPath;
        bool m_enabled = true;
        
        std::function<void()> OnStart;
        std::function<void(float)> OnUpdate;
        std::function<void()> OnDestroy;
        
        void LoadScript(const std::string& scriptPath);
        void ExecuteStart();
        void ExecuteUpdate(float deltaTime);
        void ExecuteDestroy();
        
    private:
        bool m_started = false;
    };
}
