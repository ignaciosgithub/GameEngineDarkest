#pragma once

#include <chrono>

namespace GameEngine {
    class Timer {
    public:
        static void Initialize();
        static void Update();
        
        static float GetDeltaTime();
        static float GetUnscaledDeltaTime();
        static double GetTime();
        static int GetFrameRate();
        static void SetTimeScale(float scale);
        
        static void Reset();
        
    private:
        static std::chrono::high_resolution_clock::time_point s_startTime;
        static std::chrono::high_resolution_clock::time_point s_lastFrameTime;
        static std::chrono::high_resolution_clock::time_point s_currentFrameTime;
        
        static float s_deltaTime;
        static float s_unscaledDeltaTime;
        static float s_timeScale;
        static double s_totalTime;
        
        static int s_frameCount;
        static float s_frameRateTimer;
        static int s_currentFrameRate;
        
        static bool s_initialized;
    };
}
