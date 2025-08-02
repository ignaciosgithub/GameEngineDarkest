#include "ExternalScriptManager.h"
#include "ExternalScript.h"
#include "../../Logging/Logger.h"
#include <filesystem>
#include <fstream>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace GameEngine {

ExternalScriptManager& ExternalScriptManager::Instance() {
    static ExternalScriptManager instance;
    return instance;
}

bool ExternalScriptManager::Initialize(const std::string& scriptsDirectory) {
    m_scriptsDirectory = scriptsDirectory;
    
    if (!std::filesystem::exists(m_scriptsDirectory)) {
        std::filesystem::create_directories(m_scriptsDirectory);
        Logger::Info("Created scripts directory: " + m_scriptsDirectory);
    }
    
    Logger::Info("ExternalScriptManager initialized with directory: " + m_scriptsDirectory);
    return true;
}

void ExternalScriptManager::Shutdown() {
    m_loadedScripts.clear();
    m_scriptModificationTimes.clear();
    m_entityScripts.clear();
    Logger::Info("ExternalScriptManager shutdown complete");
}

bool ExternalScriptManager::CompileScript(const std::string& scriptPath) {
    if (!std::filesystem::exists(scriptPath)) {
        Logger::Error("Script file does not exist: " + scriptPath);
        return false;
    }
    
    std::filesystem::path path(scriptPath);
    std::string scriptName = path.stem().string();
    std::string outputPath = m_scriptsDirectory + "/" + scriptName;
    
#ifdef _WIN32
    outputPath += ".dll";
#else
    outputPath += ".so";
#endif
    
    Logger::Info("Compiling script: " + scriptPath + " -> " + outputPath);
    
    if (CompileWithVisualStudio(scriptPath, outputPath)) {
        Logger::Info("Script compiled successfully: " + scriptName);
        return true;
    } else {
        Logger::Error("Failed to compile script: " + scriptName);
        return false;
    }
}

bool ExternalScriptManager::LoadCompiledScript(const std::string& scriptName) {
    std::string libraryPath = m_scriptsDirectory + "/" + scriptName;
    
#ifdef _WIN32
    libraryPath += ".dll";
#else
    libraryPath += ".so";
#endif
    
    if (!std::filesystem::exists(libraryPath)) {
        Logger::Error("Compiled script library does not exist: " + libraryPath);
        return false;
    }
    
    return LoadDynamicLibrary(libraryPath, scriptName);
}

bool ExternalScriptManager::ReloadScript(const std::string& scriptName) {
    auto it = m_loadedScripts.find(scriptName);
    if (it != m_loadedScripts.end()) {
        m_loadedScripts.erase(it);
        Logger::Info("Unloaded script for reload: " + scriptName);
    }
    
    return LoadCompiledScript(scriptName);
}

void ExternalScriptManager::ExecuteStartScripts(World* world) {
    for (const auto& [entity, scriptNames] : m_entityScripts) {
        for (const auto& scriptName : scriptNames) {
            auto it = m_loadedScripts.find(scriptName);
            if (it != m_loadedScripts.end() && it->second->IsValid()) {
                it->second->ExecuteStart(world, entity);
            }
        }
    }
}

void ExternalScriptManager::ExecuteUpdateScripts(World* world, float deltaTime) {
    for (const auto& [entity, scriptNames] : m_entityScripts) {
        for (const auto& scriptName : scriptNames) {
            auto it = m_loadedScripts.find(scriptName);
            if (it != m_loadedScripts.end() && it->second->IsValid()) {
                it->second->ExecuteUpdate(world, entity, deltaTime);
            }
        }
    }
}

void ExternalScriptManager::ExecuteDestroyScripts(World* world) {
    for (const auto& [entity, scriptNames] : m_entityScripts) {
        for (const auto& scriptName : scriptNames) {
            auto it = m_loadedScripts.find(scriptName);
            if (it != m_loadedScripts.end() && it->second->IsValid()) {
                it->second->ExecuteDestroy(world, entity);
            }
        }
    }
}

void ExternalScriptManager::CheckForScriptChanges() {
    for (const auto& [scriptName, script] : m_loadedScripts) {
        std::string scriptPath = m_scriptsDirectory + "/" + scriptName + ".cpp";
        if (IsScriptModified(scriptPath)) {
            Logger::Info("Script modified, recompiling: " + scriptName);
            if (CompileScript(scriptPath)) {
                ReloadScript(scriptName);
            }
        }
    }
}

bool ExternalScriptManager::IsScriptModified(const std::string& scriptPath) {
    struct stat fileStat;
    if (stat(scriptPath.c_str(), &fileStat) != 0) {
        return false;
    }
    
    time_t lastModified = fileStat.st_mtime;
    auto it = m_scriptModificationTimes.find(scriptPath);
    
    if (it == m_scriptModificationTimes.end()) {
        m_scriptModificationTimes[scriptPath] = lastModified;
        return false;
    }
    
    if (lastModified > it->second) {
        it->second = lastModified;
        return true;
    }
    
    return false;
}

void ExternalScriptManager::AttachScriptToEntity(Entity entity, const std::string& scriptName) {
    m_entityScripts[entity].push_back(scriptName);
    Logger::Info("Attached script '" + scriptName + "' to entity " + std::to_string(entity.GetID()));
}

void ExternalScriptManager::DetachScriptFromEntity(Entity entity, const std::string& scriptName) {
    auto it = m_entityScripts.find(entity);
    if (it != m_entityScripts.end()) {
        auto& scripts = it->second;
        scripts.erase(std::remove(scripts.begin(), scripts.end(), scriptName), scripts.end());
        
        if (scripts.empty()) {
            m_entityScripts.erase(it);
        }
        
        Logger::Info("Detached script '" + scriptName + "' from entity " + std::to_string(entity.GetID()));
    }
}

std::vector<std::string> ExternalScriptManager::GetAvailableScripts() const {
    std::vector<std::string> scripts;
    
    for (const auto& entry : std::filesystem::directory_iterator(m_scriptsDirectory)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.ends_with(".cpp")) {
                scripts.push_back(filename.substr(0, filename.length() - 4));
            }
        }
    }
    
    return scripts;
}

bool ExternalScriptManager::CompileWithVisualStudio(const std::string& scriptPath, const std::string& outputPath) {
#ifdef _WIN32
    std::string command = "cl.exe /LD /EHsc ";
    command += "/I\"" + std::filesystem::current_path().string() + "/src\" ";
    command += "\"" + scriptPath + "\" ";
    command += "/Fe:\"" + outputPath + "\" ";
    command += "/link /LIBPATH:\"" + std::filesystem::current_path().string() + "/build/Debug\" ";
    command += "Core.lib Rendering.lib";
    
    Logger::Info("Executing compile command: " + command);
    
    int result = system(command.c_str());
    return result == 0;
#else
    std::string command = "g++ -shared -fPIC -std=c++20 ";
    command += "-I" + std::filesystem::current_path().string() + "/src ";
    command += "\"" + scriptPath + "\" ";
    command += "-o \"" + outputPath + "\" ";
    command += "-L" + std::filesystem::current_path().string() + "/build ";
    command += "-lCore -lRendering -ldl";
    
    Logger::Info("Executing compile command: " + command);
    
    int result = system(command.c_str());
    return result == 0;
#endif
}

bool ExternalScriptManager::LoadDynamicLibrary(const std::string& libraryPath, const std::string& scriptName) {
#ifdef _WIN32
    HMODULE handle = LoadLibraryA(libraryPath.c_str());
    if (!handle) {
        Logger::Error("Failed to load library: " + libraryPath);
        return false;
    }
#else
    void* handle = dlopen(libraryPath.c_str(), RTLD_LAZY);
    if (!handle) {
        Logger::Error("Failed to load library: " + libraryPath + " - " + std::string(dlerror()));
        return false;
    }
#endif
    
    auto script = std::make_shared<ExternalScript>(scriptName, handle);
    if (!script->Initialize()) {
        Logger::Error("Failed to initialize script: " + scriptName);
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }
    
    m_loadedScripts[scriptName] = script;
    Logger::Info("Successfully loaded script: " + scriptName);
    return true;
}

}
