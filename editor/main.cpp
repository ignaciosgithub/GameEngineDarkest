#include "../src/Core/Engine.h"
#include "../src/Core/Project/ProjectManager.h"
#include "../src/Core/Logging/Logger.h"
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    try {
        GameEngine::Logger::Info("=== GameEngine Editor ===");
        
        if (argc > 1) {
            std::string projectPath = argv[1];
            if (std::filesystem::exists(projectPath)) {
                if (GameEngine::ProjectManager::Instance().LoadProject(projectPath)) {
                    GameEngine::Logger::Info("Loaded project: " + projectPath);
                } else {
                    GameEngine::Logger::Error("Failed to load project: " + projectPath);
                    return -1;
                }
            } else {
                GameEngine::Logger::Error("Project path does not exist: " + projectPath);
                return -1;
            }
        } else {
            GameEngine::Logger::Info("No project specified. Starting with empty editor.");
        }
        
        GameEngine::Engine engine;
        
        if (!engine.Initialize("GameEngine Editor", 1920, 1080)) {
            GameEngine::Logger::Error("Failed to initialize engine");
            return -1;
        }
        
        engine.Run();
        engine.Shutdown();
        
        return 0;
    }
    catch (const std::exception& e) {
        GameEngine::Logger::Error("Exception caught: " + std::string(e.what()));
        return -1;
    }
}
