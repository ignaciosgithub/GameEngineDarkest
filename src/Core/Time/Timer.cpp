#include "Timer.h"
#include "../Logging/Logger.h"

namespace GameEngine {
    
    std::chrono::high_resolution_clock::time_point Timer::s_startTime;
    std::chrono::high_resolution_clock::time_point Timer::s_lastFrameTime;
    std::chrono::high_resolution_clock::time_point Timer::s_currentFrameTime;
    
    float Timer::s_deltaTime = 0.0f;
    float Timer::s_unscaledDeltaTime = 0.0f;
    float Timer::s_timeScale = 1.0f;
    double Timer::s_totalTime = 0.0;
    
    int Timer::s_frameCount = 0;
    float Timer::s_frameRateTimer = 0.0f;
    int Timer::s_currentFrameRate = 0;
    
    bool Timer::s_initialized = false;
    
    void Timer::Initialize() {
        if (s_initialized) {
            Logger::Warning("Timer already initialized");
            return;
        }
        
        s_startTime = std::chrono::high_resolution_clock::now();
        s_lastFrameTime = s_startTime;
        s_currentFrameTime = s_startTime;
        
        s_deltaTime = 0.0f;
        s_unscaledDeltaTime = 0.0f;
        s_timeScale = 1.0f;
        s_totalTime = 0.0;
        
        s_frameCount = 0;
        s_frameRateTimer = 0.0f;
        s_currentFrameRate = 0;
        
        s_initialized = true;
        Logger::Info("Timer system initialized");
    }
    
    void Timer::Update() {
        if (!s_initialized) {
            Logger::Error("Timer not initialized - call Timer::Initialize() first");
            return;
        }
        
        s_currentFrameTime = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            s_currentFrameTime - s_lastFrameTime
        );
        
        s_unscaledDeltaTime = duration.count() / 1000000.0f;
        s_deltaTime = s_unscaledDeltaTime * s_timeScale;
        
        s_totalTime += s_unscaledDeltaTime;
        
        s_frameCount++;
        s_frameRateTimer += s_unscaledDeltaTime;
        
        if (s_frameRateTimer >= 1.0f) {
            s_currentFrameRate = s_frameCount;
            s_frameCount = 0;
            s_frameRateTimer = 0.0f;
        }
        
        s_lastFrameTime = s_currentFrameTime;
    }
    
    float Timer::GetDeltaTime() {
        return s_deltaTime;
    }
    
    float Timer::GetUnscaledDeltaTime() {
        return s_unscaledDeltaTime;
    }
    
    double Timer::GetTime() {
        return s_totalTime;
    }
    
    int Timer::GetFrameRate() {
        return s_currentFrameRate;
    }
    
    void Timer::SetTimeScale(float scale) {
        if (scale < 0.0f) {
            Logger::Warning("Time scale cannot be negative, clamping to 0.0");
            scale = 0.0f;
        }
        
        s_timeScale = scale;
        Logger::Debug("Time scale set to: " + std::to_string(scale));
    }
    
    void Timer::Reset() {
        if (!s_initialized) {
            Logger::Warning("Timer not initialized - cannot reset");
            return;
        }
        
        s_startTime = std::chrono::high_resolution_clock::now();
        s_lastFrameTime = s_startTime;
        s_currentFrameTime = s_startTime;
        
        s_deltaTime = 0.0f;
        s_unscaledDeltaTime = 0.0f;
        s_totalTime = 0.0;
        
        s_frameCount = 0;
        s_frameRateTimer = 0.0f;
        s_currentFrameRate = 0;
        
        Logger::Info("Timer system reset");
    }
}
