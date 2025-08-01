#include "ScriptComponent.h"
#include "../Logging/Logger.h"

namespace GameEngine {

void ScriptComponent::LoadScript(const std::string& scriptPath) {
    m_scriptPath = scriptPath;
    m_started = false;
    
    Logger::Info("Script loaded: " + scriptPath);
}

void ScriptComponent::ExecuteStart() {
    if (m_started || !m_enabled) return;
    
    if (OnStart) {
        try {
            OnStart();
            m_started = true;
            Logger::Debug("Script Start executed: " + m_scriptPath);
        }
        catch (const std::exception& e) {
            Logger::Error("Script Start error in " + m_scriptPath + ": " + e.what());
        }
    }
}

void ScriptComponent::ExecuteUpdate(float deltaTime) {
    if (!m_started || !m_enabled) return;
    
    if (OnUpdate) {
        try {
            OnUpdate(deltaTime);
        }
        catch (const std::exception& e) {
            Logger::Error("Script Update error in " + m_scriptPath + ": " + e.what());
        }
    }
}

void ScriptComponent::ExecuteDestroy() {
    if (!m_started || !m_enabled) return;
    
    if (OnDestroy) {
        try {
            OnDestroy();
            Logger::Debug("Script Destroy executed: " + m_scriptPath);
        }
        catch (const std::exception& e) {
            Logger::Error("Script Destroy error in " + m_scriptPath + ": " + e.what());
        }
    }
    
    m_started = false;
}

}
