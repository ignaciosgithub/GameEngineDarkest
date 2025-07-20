#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace GameEngine {

std::unique_ptr<std::ofstream> Logger::s_fileStream = nullptr;
LogLevel Logger::s_logLevel = LogLevel::Info;
bool Logger::s_consoleOutput = true;
bool Logger::s_fileOutput = true;
bool Logger::s_initialized = false;

void Logger::Initialize(const std::string& filename, LogLevel level) {
    if (s_initialized) {
        Shutdown();
    }
    
    s_logLevel = level;
    
    if (s_fileOutput) {
        s_fileStream = std::make_unique<std::ofstream>(filename, std::ios::out | std::ios::app);
        if (!s_fileStream->is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
            s_fileOutput = false;
        }
    }
    
    s_initialized = true;
    Info("Logger initialized");
}

void Logger::Shutdown() {
    if (s_initialized) {
        Info("Logger shutting down");
        
        if (s_fileStream) {
            s_fileStream->close();
            s_fileStream.reset();
        }
        
        s_initialized = false;
    }
}

void Logger::Debug(const std::string& message) {
    Log(LogLevel::Debug, message);
}

void Logger::Info(const std::string& message) {
    Log(LogLevel::Info, message);
}

void Logger::Warning(const std::string& message) {
    Log(LogLevel::Warning, message);
}

void Logger::Error(const std::string& message) {
    Log(LogLevel::Error, message);
}

void Logger::SetLogLevel(LogLevel level) {
    s_logLevel = level;
}

LogLevel Logger::GetLogLevel() {
    return s_logLevel;
}

void Logger::EnableConsoleOutput(bool enable) {
    s_consoleOutput = enable;
}

void Logger::EnableFileOutput(bool enable) {
    s_fileOutput = enable;
}

void Logger::Log(LogLevel level, const std::string& message) {
    if (!s_initialized && level >= LogLevel::Warning) {
        Initialize();
    }
    
    if (level < s_logLevel) {
        return;
    }
    
    std::string timestamp = GetTimestamp();
    std::string levelStr = LogLevelToString(level);
    std::string logMessage = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    if (s_consoleOutput) {
        if (level >= LogLevel::Error) {
            std::cerr << logMessage << std::endl;
        } else {
            std::cout << logMessage << std::endl;
        }
    }
    
    if (s_fileOutput && s_fileStream && s_fileStream->is_open()) {
        *s_fileStream << logMessage << std::endl;
        s_fileStream->flush();
    }
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
#ifdef _WIN32
    struct tm timeinfo;
    localtime_s(&timeinfo, &time_t);
    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
#else
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
#endif
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

}
