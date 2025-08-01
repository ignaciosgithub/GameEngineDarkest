#pragma once

#include <string>
#include <memory>
#include <vector>

namespace GameEngine {
    class World;
    class Scene;
    
    struct ProjectSettings {
        std::string name;
        std::string version = "1.0.0";
        std::string engineVersion;
        std::string startScene;
        std::vector<std::string> scenes;
        std::string assetsPath = "Assets/";
        std::string buildPath = "Build/";
    };
    
    class ProjectManager {
    public:
        static ProjectManager& Instance();
        
        bool CreateProject(const std::string& projectPath, const std::string& projectName);
        bool LoadProject(const std::string& projectPath);
        bool SaveProject();
        void CloseProject();
        
        bool IsProjectLoaded() const { return m_projectLoaded; }
        const ProjectSettings& GetSettings() const { return m_settings; }
        const std::string& GetProjectPath() const { return m_projectPath; }
        
        void AddScene(const std::string& scenePath);
        void RemoveScene(const std::string& scenePath);
        
    private:
        ProjectManager() = default;
        bool LoadProjectSettings();
        bool SaveProjectSettings();
        void CreateProjectStructure();
        
        ProjectSettings m_settings;
        std::string m_projectPath;
        bool m_projectLoaded = false;
    };
}
