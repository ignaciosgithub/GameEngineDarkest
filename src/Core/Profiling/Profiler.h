#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>

namespace GameEngine {

struct ProfilerSample {
    std::string name;
    double startTime;
    double endTime;
    double duration;
    std::thread::id threadId;
    int frameNumber;
    
    ProfilerSample() : startTime(0.0), endTime(0.0), duration(0.0), frameNumber(0) {}
};

struct ProfilerStats {
    std::string name;
    double totalTime = 0.0;
    double averageTime = 0.0;
    double minTime = std::numeric_limits<double>::max();
    double maxTime = 0.0;
    int sampleCount = 0;
    double lastFrameTime = 0.0;
    
    void AddSample(double time) {
        totalTime += time;
        sampleCount++;
        averageTime = totalTime / sampleCount;
        minTime = std::min(minTime, time);
        maxTime = std::max(maxTime, time);
        lastFrameTime = time;
    }
    
    void Reset() {
        totalTime = 0.0;
        averageTime = 0.0;
        minTime = std::numeric_limits<double>::max();
        maxTime = 0.0;
        sampleCount = 0;
        lastFrameTime = 0.0;
    }
};

struct GPUProfilerSample {
    std::string name;
    unsigned int queryStart = 0;
    unsigned int queryEnd = 0;
    double gpuTime = 0.0;
    bool completed = false;
};

class Profiler {
public:
    static void Initialize();
    static void Shutdown();
    
    static void BeginFrame();
    static void EndFrame();
    
    static void BeginSample(const std::string& name);
    static void EndSample(const std::string& name);
    
    static void BeginGPUSample(const std::string& name);
    static void EndGPUSample(const std::string& name);
    
    static void SetEnabled(bool enabled) { s_enabled = enabled; }
    static bool IsEnabled() { return s_enabled; }
    
    static const std::unordered_map<std::string, ProfilerStats>& GetStats() { return s_stats; }
    static const std::vector<ProfilerSample>& GetFrameSamples() { return s_frameSamples; }
    static const std::unordered_map<std::string, GPUProfilerSample>& GetGPUSamples() { return s_gpuSamples; }
    
    static void PrintFrameReport();
    static void PrintDetailedReport();
    static void SaveReportToFile(const std::string& filename);
    
    static double GetFrameTime() { return s_frameTime; }
    static int GetCurrentFrameNumber() { return s_frameNumber; }
    
    static void ResetStats();
    
private:
    static bool s_initialized;
    static bool s_enabled;
    static std::atomic<int> s_frameNumber;
    static double s_frameStartTime;
    static double s_frameTime;
    
    static std::unordered_map<std::string, double> s_activeSamples;
    static std::unordered_map<std::string, ProfilerStats> s_stats;
    static std::vector<ProfilerSample> s_frameSamples;
    static std::unordered_map<std::string, GPUProfilerSample> s_gpuSamples;
    
    static std::mutex s_mutex;
    static std::chrono::high_resolution_clock::time_point s_startTime;
    
    static double GetTickCount();
    static void ProcessGPUQueries();
};

class ScopedProfiler {
public:
    explicit ScopedProfiler(const std::string& name) : m_name(name) {
        if (Profiler::IsEnabled()) {
            Profiler::BeginSample(m_name);
        }
    }
    
    ~ScopedProfiler() {
        if (Profiler::IsEnabled()) {
            Profiler::EndSample(m_name);
        }
    }
    
private:
    std::string m_name;
};

class ScopedGPUProfiler {
public:
    explicit ScopedGPUProfiler(const std::string& name) : m_name(name) {
        if (Profiler::IsEnabled()) {
            Profiler::BeginGPUSample(m_name);
        }
    }
    
    ~ScopedGPUProfiler() {
        if (Profiler::IsEnabled()) {
            Profiler::EndGPUSample(m_name);
        }
    }
    
private:
    std::string m_name;
};

// Helper macros for creating unique variable names
#define GE_CONCAT_INNER(x,y) x##y
#define GE_CONCAT(x,y) GE_CONCAT_INNER(x,y)
#ifdef _MSC_VER
#define GE_UNIQUE(base) GE_CONCAT(base, __COUNTER__)
#else
#define GE_UNIQUE(base) GE_CONCAT(base, __LINE__)
#endif

#define PROFILE_SCOPE(name) ScopedProfiler GE_UNIQUE(_prof)(name)
#define PROFILE_FUNCTION() ScopedProfiler GE_UNIQUE(_prof)(__FUNCTION__)
#define PROFILE_GPU(name) ScopedGPUProfiler GE_UNIQUE(_gpuProf)(name)

}
