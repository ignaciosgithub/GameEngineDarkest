#pragma once

#include "../../ECS/World.h"
#include "../../ECS/Entity.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace GameEngine {
    class ExternalScript;
    
    class ExternalScriptManager {
    public:
        static ExternalScriptManager& Instance();
        
        bool Initialize(const std::string& scriptsDirectory = "Scripts/");
        void Shutdown();
        
        bool CompileScript(const std::string& scriptPath);
        bool LoadCompiledScript(const std::string& scriptName);
        bool ReloadScript(const std::string& scriptName);
        
        void ExecuteStartScripts(World* world);
        void ExecuteUpdateScripts(World* world, float deltaTime);
        void ExecuteDestroyScripts(World* world);
        
        void CheckForScriptChanges();
        bool IsScriptModified(const std::string& scriptPath);
        
        void AttachScriptToEntity(Entity entity, const std::string& scriptName);
        void DetachScriptFromEntity(Entity entity, const std::string& scriptName);
        
        const std::string& GetScriptsDirectory() const { return m_scriptsDirectory; }
        std::vector<std::string> GetAvailableScripts() const;
        
    private:
        ExternalScriptManager() = default;
        ~ExternalScriptManager() = default;
        
        std::string m_scriptsDirectory;
        std::unordered_map<std::string, std::shared_ptr<ExternalScript>> m_loadedScripts;
        std::unordered_map<std::string, time_t> m_scriptModificationTimes;
        std::unordered_map<Entity, std::vector<std::string>> m_entityScripts;
        
        bool CompileWithVisualStudio(const std::string& scriptPath, const std::string& outputPath);
        bool LoadDynamicLibrary(const std::string& libraryPath, const std::string& scriptName);
    };
}
