#include "Core/Engine.h"
#include "Core/Logging/Logger.h"
#include <iostream>

int main() {
    try {
        GameEngine::Engine engine;
        
        if (!engine.Initialize("GameEngine Demo", 1280, 720)) {
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
    catch (...) {
        GameEngine::Logger::Error("Unknown exception caught");
        return -1;
    }
}
