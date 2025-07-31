#include "Core/Engine.h"
#include "Core/Logging/Logger.h"
#include <iostream>

int main() {
    try {
        GameEngine::Engine engine;
        
        if (!engine.Initialize("GameEngine Multi-Light Occlusion Demo", 1280, 720)) {
            GameEngine::Logger::Error("Failed to initialize engine");
            return -1;
        }
        
        GameEngine::Logger::Info("=== Starting Multi-Light Demo ===");
        GameEngine::Logger::Info("Features demonstrated:");
        GameEngine::Logger::Info("- Multiple point lights with different colors and intensities");
        GameEngine::Logger::Info("- Real-time light property changes and animations");
        GameEngine::Logger::Info("- Light occlusion through collision geometry");
        GameEngine::Logger::Info("- Light accumulation with brightness clamping (MAX_BRIGHTNESS = 100.0f)");
        GameEngine::Logger::Info("- Point light shadows and directional lighting");
        GameEngine::Logger::Info("- Forward and Deferred rendering pipeline support");
        GameEngine::Logger::Info("");
        GameEngine::Logger::Info("Controls:");
        GameEngine::Logger::Info("- WASD: Move camera");
        GameEngine::Logger::Info("- Mouse: Look around");
        GameEngine::Logger::Info("- 1: Switch to BasicLighting scene with Deferred rendering");
        GameEngine::Logger::Info("- 2: Switch to MultipleLight scene with Forward rendering");
        GameEngine::Logger::Info("- 3: Switch to PBRMaterials scene with Deferred rendering");
        GameEngine::Logger::Info("- 4: Switch to PostProcessing scene with Forward rendering");
        GameEngine::Logger::Info("- 5: Switch to Raytracing scene with Raytracing pipeline");
        GameEngine::Logger::Info("- ESC: Exit demo");
        GameEngine::Logger::Info("");
        GameEngine::Logger::Info("The engine automatically creates a 5x5 cube grid with multiple lights:");
        GameEngine::Logger::Info("- Dynamic point lights with different colors (red, green, blue)");
        GameEngine::Logger::Info("- Animated light movement and intensity changes");
        GameEngine::Logger::Info("- Directional sun light for ambient illumination");
        GameEngine::Logger::Info("- Light occlusion system preventing light bleeding through walls");
        GameEngine::Logger::Info("- Shadow mapping for realistic lighting effects");
        
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
