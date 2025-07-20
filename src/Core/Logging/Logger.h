#pragma once

#include <string>
#include <fstream>
#include <memory>

namespace GameEngine {
    enum class LogLevel {
        Debug = 0,
        Info = 1,
        Warning = 2,
        Error = 3
    };
    
    class Logger {
    public:
        static void Initialize(const std::string& filename = "engine.log", LogLevel level = LogLevel::Info);
        static void Shutdown();
        
        static void Debug(const std::string& message);
        static void Info(const std::string& message);
        static void Warning(const std::string& message);
        static void Error(const std::string& message);
        
        static void SetLogLevel(LogLevel level);
        static LogLevel GetLogLevel();
        
        static void EnableConsoleOutput(bool enable);
        static void EnableFileOutput(bool enable);
        
    private:
        static void Log(LogLevel level, const std::string& message);
        static std::string GetTimestamp();
        static std::string LogLevelToString(LogLevel level);
        
        static std::unique_ptr<std::ofstream> s_fileStream;
        static LogLevel s_logLevel;
        static bool s_consoleOutput;
        static bool s_fileOutput;
        static bool s_initialized;
    };
}
