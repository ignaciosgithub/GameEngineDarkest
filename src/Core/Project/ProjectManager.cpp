#include "ProjectManager.h"
#include "../Logging/Logger.h"
#include <fstream>
#include <filesystem>
#include <sstream>

namespace GameEngine {

ProjectManager& ProjectManager::Instance() {
    static ProjectManager instance;
    return instance;
}

bool ProjectManager::CreateProject(const std::string& projectPath, const std::string& projectName) {
    if (m_projectLoaded) {
        Logger::Warning("A project is already loaded. Close it first.");
        return false;
    }
    
    m_projectPath = projectPath;
    m_settings.name = projectName;
    m_settings.engineVersion = "1.0.0";
    m_settings.startScene = "main.scene";
    
    try {
        CreateProjectStructure();
        
        if (!SaveProjectSettings()) {
            Logger::Error("Failed to save project settings");
            return false;
        }
        
        m_projectLoaded = true;
        Logger::Info("Created new project: " + projectName + " at " + projectPath);
        return true;
    }
    catch (const std::exception& e) {
        Logger::Error("Failed to create project: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::LoadProject(const std::string& projectPath) {
    if (m_projectLoaded) {
        Logger::Warning("A project is already loaded. Close it first.");
        return false;
    }
    
    m_projectPath = projectPath;
    
    if (!LoadProjectSettings()) {
        Logger::Error("Failed to load project settings from: " + projectPath);
        return false;
    }
    
    m_projectLoaded = true;
    Logger::Info("Loaded project: " + m_settings.name + " from " + projectPath);
    return true;
}

bool ProjectManager::SaveProject() {
    if (!m_projectLoaded) {
        Logger::Warning("No project loaded to save");
        return false;
    }
    
    return SaveProjectSettings();
}

void ProjectManager::CloseProject() {
    if (!m_projectLoaded) {
        Logger::Warning("No project loaded to close");
        return;
    }
    
    m_settings = ProjectSettings{};
    m_projectPath.clear();
    m_projectLoaded = false;
    
    Logger::Info("Project closed");
}

void ProjectManager::AddScene(const std::string& scenePath) {
    if (!m_projectLoaded) {
        Logger::Warning("No project loaded");
        return;
    }
    
    auto it = std::find(m_settings.scenes.begin(), m_settings.scenes.end(), scenePath);
    if (it == m_settings.scenes.end()) {
        m_settings.scenes.push_back(scenePath);
        Logger::Info("Added scene to project: " + scenePath);
    }
}

void ProjectManager::RemoveScene(const std::string& scenePath) {
    if (!m_projectLoaded) {
        Logger::Warning("No project loaded");
        return;
    }
    
    auto it = std::find(m_settings.scenes.begin(), m_settings.scenes.end(), scenePath);
    if (it != m_settings.scenes.end()) {
        m_settings.scenes.erase(it);
        Logger::Info("Removed scene from project: " + scenePath);
    }
}

bool ProjectManager::LoadProjectSettings() {
    std::string settingsPath = m_projectPath + "/project.json";
    
    if (!std::filesystem::exists(settingsPath)) {
        Logger::Error("Project settings file not found: " + settingsPath);
        return false;
    }
    
    std::ifstream file(settingsPath);
    if (!file.is_open()) {
        Logger::Error("Failed to open project settings file: " + settingsPath);
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("\"name\":") != std::string::npos) {
            size_t start = line.find("\"", line.find(":") + 1) + 1;
            size_t end = line.find("\"", start);
            if (start != std::string::npos && end != std::string::npos) {
                m_settings.name = line.substr(start, end - start);
            }
        }
        else if (line.find("\"version\":") != std::string::npos) {
            size_t start = line.find("\"", line.find(":") + 1) + 1;
            size_t end = line.find("\"", start);
            if (start != std::string::npos && end != std::string::npos) {
                m_settings.version = line.substr(start, end - start);
            }
        }
        else if (line.find("\"engineVersion\":") != std::string::npos) {
            size_t start = line.find("\"", line.find(":") + 1) + 1;
            size_t end = line.find("\"", start);
            if (start != std::string::npos && end != std::string::npos) {
                m_settings.engineVersion = line.substr(start, end - start);
            }
        }
        else if (line.find("\"startScene\":") != std::string::npos) {
            size_t start = line.find("\"", line.find(":") + 1) + 1;
            size_t end = line.find("\"", start);
            if (start != std::string::npos && end != std::string::npos) {
                m_settings.startScene = line.substr(start, end - start);
            }
        }
        else if (line.find("\"assetsPath\":") != std::string::npos) {
            size_t start = line.find("\"", line.find(":") + 1) + 1;
            size_t end = line.find("\"", start);
            if (start != std::string::npos && end != std::string::npos) {
                m_settings.assetsPath = line.substr(start, end - start);
            }
        }
        else if (line.find("\"buildPath\":") != std::string::npos) {
            size_t start = line.find("\"", line.find(":") + 1) + 1;
            size_t end = line.find("\"", start);
            if (start != std::string::npos && end != std::string::npos) {
                m_settings.buildPath = line.substr(start, end - start);
            }
        }
    }
    
    file.close();
    return true;
}

bool ProjectManager::SaveProjectSettings() {
    std::string settingsPath = m_projectPath + "/project.json";
    
    std::ofstream file(settingsPath);
    if (!file.is_open()) {
        Logger::Error("Failed to create project settings file: " + settingsPath);
        return false;
    }
    
    file << "{\n";
    file << "  \"name\": \"" << m_settings.name << "\",\n";
    file << "  \"version\": \"" << m_settings.version << "\",\n";
    file << "  \"engineVersion\": \"" << m_settings.engineVersion << "\",\n";
    file << "  \"startScene\": \"" << m_settings.startScene << "\",\n";
    file << "  \"assetsPath\": \"" << m_settings.assetsPath << "\",\n";
    file << "  \"buildPath\": \"" << m_settings.buildPath << "\",\n";
    file << "  \"scenes\": [\n";
    
    for (size_t i = 0; i < m_settings.scenes.size(); ++i) {
        file << "    \"" << m_settings.scenes[i] << "\"";
        if (i < m_settings.scenes.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    Logger::Info("Saved project settings to: " + settingsPath);
    return true;
}

std::vector<std::string> ProjectManager::GetAssetList() const {
    std::vector<std::string> assets;
    
    if (!m_projectLoaded) {
        Logger::Warning("No project loaded");
        return assets;
    }
    
    std::string assetsDir = m_projectPath + "/" + m_settings.assetsPath;
    
    if (!std::filesystem::exists(assetsDir)) {
        return assets;
    }
    
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(assetsDir)) {
            if (entry.is_regular_file()) {
                std::string relativePath = std::filesystem::relative(entry.path(), assetsDir).string();
                assets.push_back(relativePath);
            }
        }
    } catch (const std::exception& e) {
        Logger::Error("Failed to scan assets directory: " + std::string(e.what()));
    }
    
    return assets;
}

bool ProjectManager::ImportAsset(const std::string& sourcePath, const std::string& destinationPath) {
    if (!m_projectLoaded) {
        Logger::Warning("No project loaded");
        return false;
    }
    
    try {
        std::filesystem::path source(sourcePath);
        std::filesystem::path destination(m_projectPath + "/" + m_settings.assetsPath + "/" + destinationPath);
        
        if (!std::filesystem::exists(source)) {
            Logger::Error("Source file does not exist: " + sourcePath);
            return false;
        }
        
        std::filesystem::create_directories(destination.parent_path());
        std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing);
        
        Logger::Info("Asset imported: " + destinationPath);
        return true;
    } catch (const std::exception& e) {
        Logger::Error("Failed to import asset: " + std::string(e.what()));
        return false;
    }
}

bool ProjectManager::DeleteAsset(const std::string& assetPath) {
    if (!m_projectLoaded) {
        Logger::Warning("No project loaded");
        return false;
    }
    
    try {
        std::filesystem::path fullPath(m_projectPath + "/" + m_settings.assetsPath + "/" + assetPath);
        
        if (!std::filesystem::exists(fullPath)) {
            Logger::Error("Asset does not exist: " + assetPath);
            return false;
        }
        
        std::filesystem::remove(fullPath);
        Logger::Info("Asset deleted: " + assetPath);
        return true;
    } catch (const std::exception& e) {
        Logger::Error("Failed to delete asset: " + std::string(e.what()));
        return false;
    }
}

std::string ProjectManager::GetAssetsDirectory() const {
    if (!m_projectLoaded) {
        return "";
    }
    
    return m_projectPath + "/" + m_settings.assetsPath;
}

void ProjectManager::CreateProjectStructure() {
    std::filesystem::create_directories(m_projectPath);
    std::filesystem::create_directories(m_projectPath + "/" + m_settings.assetsPath);
    std::filesystem::create_directories(m_projectPath + "/" + m_settings.assetsPath + "/Models");
    std::filesystem::create_directories(m_projectPath + "/" + m_settings.assetsPath + "/Textures");
    std::filesystem::create_directories(m_projectPath + "/" + m_settings.assetsPath + "/Sounds");
    std::filesystem::create_directories(m_projectPath + "/" + m_settings.assetsPath + "/Scripts");
    std::filesystem::create_directories(m_projectPath + "/" + m_settings.buildPath);
    
    Logger::Info("Created project directory structure at: " + m_projectPath);
}

}
