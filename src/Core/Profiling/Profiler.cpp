#include "Profiler.h"
#include "../Logging/Logger.h"
#include "../../Rendering/Core/OpenGLHeaders.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace GameEngine {

bool Profiler::s_initialized = false;
bool Profiler::s_enabled = true;
std::atomic<int> Profiler::s_frameNumber(0);
double Profiler::s_frameStartTime = 0.0;
double Profiler::s_frameTime = 0.0;

std::unordered_map<std::string, double> Profiler::s_activeSamples;
std::unordered_map<std::string, ProfilerStats> Profiler::s_stats;
std::vector<ProfilerSample> Profiler::s_frameSamples;
std::unordered_map<std::string, GPUProfilerSample> Profiler::s_gpuSamples;

std::mutex Profiler::s_mutex;
std::chrono::high_resolution_clock::time_point Profiler::s_startTime;

void Profiler::Initialize() {
    if (s_initialized) {
        Logger::Warning("Profiler already initialized");
        return;
    }
    
    s_startTime = std::chrono::high_resolution_clock::now();
    s_frameNumber = 0;
    s_frameStartTime = 0.0;
    s_frameTime = 0.0;
    
    s_activeSamples.clear();
    s_stats.clear();
    s_frameSamples.clear();
    s_gpuSamples.clear();
    
    s_initialized = true;
    s_enabled = true;
    
    Logger::Info("Profiler system initialized");
}

void Profiler::Shutdown() {
    if (!s_initialized) return;
    
    PrintDetailedReport();
    
    s_activeSamples.clear();
    s_stats.clear();
    s_frameSamples.clear();
    s_gpuSamples.clear();
    
    s_initialized = false;
    Logger::Info("Profiler system shutdown");
}

void Profiler::BeginFrame() {
    if (!s_enabled || !s_initialized) return;
    
    std::lock_guard<std::mutex> lock(s_mutex);
    
    s_frameStartTime = GetTickCount();
    s_frameSamples.clear();
    
    ProcessGPUQueries();
}

void Profiler::EndFrame() {
    if (!s_enabled || !s_initialized) return;
    
    std::lock_guard<std::mutex> lock(s_mutex);
    
    double frameEndTime = GetTickCount();
    s_frameTime = frameEndTime - s_frameStartTime;
    
    ProfilerStats& frameStats = s_stats["Frame"];
    frameStats.AddSample(s_frameTime);
    
    s_frameNumber++;
    
    if (s_frameNumber % 60 == 0) {
        PrintFrameReport();
    }
}

void Profiler::BeginSample(const std::string& name) {
    if (!s_enabled || !s_initialized) return;
    
    std::lock_guard<std::mutex> lock(s_mutex);
    s_activeSamples[name] = GetTickCount();
}

void Profiler::EndSample(const std::string& name) {
    if (!s_enabled || !s_initialized) return;
    
    double endTime = GetTickCount();
    
    std::lock_guard<std::mutex> lock(s_mutex);
    
    auto it = s_activeSamples.find(name);
    if (it == s_activeSamples.end()) {
        Logger::Warning("EndSample called for '" + name + "' without matching BeginSample");
        return;
    }
    
    double startTime = it->second;
    double duration = endTime - startTime;
    
    ProfilerSample sample;
    sample.name = name;
    sample.startTime = startTime;
    sample.endTime = endTime;
    sample.duration = duration;
    sample.threadId = std::this_thread::get_id();
    sample.frameNumber = s_frameNumber;
    
    s_frameSamples.push_back(sample);
    
    ProfilerStats& stats = s_stats[name];
    stats.name = name;
    stats.AddSample(duration);
    
    s_activeSamples.erase(it);
}

void Profiler::BeginGPUSample(const std::string& name) {
    if (!s_enabled || !s_initialized) return;
    
    std::lock_guard<std::mutex> lock(s_mutex);
    
    GPUProfilerSample& sample = s_gpuSamples[name];
    sample.name = name;
    sample.completed = false;
    
    if (sample.queryStart == 0) {
        glGenQueries(1, &sample.queryStart);
        glGenQueries(1, &sample.queryEnd);
    }
    
    glQueryCounter(sample.queryStart, GL_TIMESTAMP);
}

void Profiler::EndGPUSample(const std::string& name) {
    if (!s_enabled || !s_initialized) return;
    
    std::lock_guard<std::mutex> lock(s_mutex);
    
    auto it = s_gpuSamples.find(name);
    if (it == s_gpuSamples.end()) {
        Logger::Warning("EndGPUSample called for '" + name + "' without matching BeginGPUSample");
        return;
    }
    
    glQueryCounter(it->second.queryEnd, GL_TIMESTAMP);
}

void Profiler::ProcessGPUQueries() {
    for (auto& pair : s_gpuSamples) {
        GPUProfilerSample& sample = pair.second;
        
        if (sample.queryStart == 0 || sample.queryEnd == 0) continue;
        
        GLint startAvailable = 0, endAvailable = 0;
        glGetQueryObjectiv(sample.queryStart, GL_QUERY_RESULT_AVAILABLE, &startAvailable);
        glGetQueryObjectiv(sample.queryEnd, GL_QUERY_RESULT_AVAILABLE, &endAvailable);
        
        if (startAvailable && endAvailable) {
            GLuint64 startTime, endTime;
            glGetQueryObjectui64v(sample.queryStart, GL_QUERY_RESULT, &startTime);
            glGetQueryObjectui64v(sample.queryEnd, GL_QUERY_RESULT, &endTime);
            
            sample.gpuTime = (endTime - startTime) / 1000000.0;
            sample.completed = true;
            
            ProfilerStats& stats = s_stats["GPU_" + sample.name];
            stats.name = "GPU_" + sample.name;
            stats.AddSample(sample.gpuTime);
        }
    }
}

void Profiler::PrintFrameReport() {
    if (!s_enabled || !s_initialized) return;
    
    std::stringstream ss;
    ss << "\n=== PROFILER FRAME REPORT (Frame " << s_frameNumber << ") ===\n";
    ss << std::fixed << std::setprecision(3);
    ss << "Frame Time: " << s_frameTime * 1000.0 << "ms (" << (1.0 / s_frameTime) << " FPS)\n";
    
    std::vector<std::pair<std::string, ProfilerStats*>> sortedStats;
    for (auto& pair : s_stats) {
        if (pair.first != "Frame") {
            sortedStats.push_back({pair.first, &pair.second});
        }
    }
    
    std::sort(sortedStats.begin(), sortedStats.end(), 
        [](const auto& a, const auto& b) {
            return a.second->lastFrameTime > b.second->lastFrameTime;
        });
    
    ss << "\nTop Performance Consumers (Last Frame):\n";
    ss << std::setw(25) << "System" << std::setw(12) << "Time (ms)" << std::setw(12) << "% of Frame\n";
    ss << std::string(50, '-') << "\n";
    
    for (size_t i = 0; i < std::min(size_t(10), sortedStats.size()); ++i) {
        const auto& stat = *sortedStats[i].second;
        double percentage = (stat.lastFrameTime / s_frameTime) * 100.0;
        ss << std::setw(25) << stat.name 
           << std::setw(12) << stat.lastFrameTime * 1000.0
           << std::setw(11) << percentage << "%\n";
    }
    
    Logger::Info(ss.str());
}

void Profiler::PrintDetailedReport() {
    if (!s_enabled || !s_initialized) return;
    
    std::stringstream ss;
    ss << "\n=== DETAILED PROFILER REPORT ===\n";
    ss << std::fixed << std::setprecision(3);
    ss << "Total Frames: " << s_frameNumber << "\n";
    
    if (s_stats.find("Frame") != s_stats.end()) {
        const auto& frameStats = s_stats.at("Frame");
        ss << "Average Frame Time: " << frameStats.averageTime * 1000.0 << "ms\n";
        ss << "Average FPS: " << (1.0 / frameStats.averageTime) << "\n";
        ss << "Min Frame Time: " << frameStats.minTime * 1000.0 << "ms\n";
        ss << "Max Frame Time: " << frameStats.maxTime * 1000.0 << "ms\n";
    }
    
    ss << "\nDetailed System Performance:\n";
    ss << std::setw(25) << "System" << std::setw(12) << "Avg (ms)" << std::setw(12) << "Min (ms)" 
       << std::setw(12) << "Max (ms)" << std::setw(10) << "Samples\n";
    ss << std::string(70, '-') << "\n";
    
    std::vector<std::pair<std::string, ProfilerStats*>> sortedStats;
    for (auto& pair : s_stats) {
        sortedStats.push_back({pair.first, &pair.second});
    }
    
    std::sort(sortedStats.begin(), sortedStats.end(), 
        [](const auto& a, const auto& b) {
            return a.second->averageTime > b.second->averageTime;
        });
    
    for (const auto& pair : sortedStats) {
        const auto& stat = *pair.second;
        ss << std::setw(25) << stat.name 
           << std::setw(12) << stat.averageTime * 1000.0
           << std::setw(12) << stat.minTime * 1000.0
           << std::setw(12) << stat.maxTime * 1000.0
           << std::setw(10) << stat.sampleCount << "\n";
    }
    
    Logger::Info(ss.str());
}

void Profiler::SaveReportToFile(const std::string& filename) {
    if (!s_enabled || !s_initialized) return;
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        Logger::Error("Failed to open profiler report file: " + filename);
        return;
    }
    
    file << "GameEngineDarkest Profiler Report\n";
    file << "Generated at frame: " << s_frameNumber << "\n";
    file << "=================================\n\n";
    
    if (s_stats.find("Frame") != s_stats.end()) {
        const auto& frameStats = s_stats.at("Frame");
        file << "Frame Statistics:\n";
        file << "  Average Frame Time: " << frameStats.averageTime * 1000.0 << "ms\n";
        file << "  Average FPS: " << (1.0 / frameStats.averageTime) << "\n";
        file << "  Min Frame Time: " << frameStats.minTime * 1000.0 << "ms\n";
        file << "  Max Frame Time: " << frameStats.maxTime * 1000.0 << "ms\n\n";
    }
    
    file << "System Performance Breakdown:\n";
    file << "System Name,Average Time (ms),Min Time (ms),Max Time (ms),Sample Count\n";
    
    for (const auto& pair : s_stats) {
        const auto& stat = pair.second;
        file << stat.name << "," 
             << stat.averageTime * 1000.0 << ","
             << stat.minTime * 1000.0 << ","
             << stat.maxTime * 1000.0 << ","
             << stat.sampleCount << "\n";
    }
    
    file.close();
    Logger::Info("Profiler report saved to: " + filename);
}

void Profiler::ResetStats() {
    if (!s_initialized) return;
    
    std::lock_guard<std::mutex> lock(s_mutex);
    
    for (auto& pair : s_stats) {
        pair.second.Reset();
    }
    
    s_frameNumber = 0;
    Logger::Info("Profiler statistics reset");
}

double Profiler::GetTickCount() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - s_startTime);
    return duration.count() / 1000000.0;
}

}
