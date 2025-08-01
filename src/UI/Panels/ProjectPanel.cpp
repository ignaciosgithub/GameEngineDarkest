#include "ProjectPanel.h"
#include "../../Core/Logging/Logger.h"
#include "../../Core/Project/ProjectManager.h"
#include <imgui.h>
#include <filesystem>

namespace GameEngine {

ProjectPanel::ProjectPanel() = default;

ProjectPanel::~ProjectPanel() = default;

void ProjectPanel::Update(World* /*world*/, float /*deltaTime*/) {
    if (!m_visible) return;
    
    if (ImGui::Begin("Project", &m_visible)) {
        ImGui::Text("Assets");
        ImGui::Separator();
        
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Import Asset...")) {
                OpenAssetImportDialog();
            }
            if (ImGui::MenuItem("Create Folder")) {
                Logger::Info("Create Folder requested");
            }
            ImGui::EndPopup();
        }
        
        auto& projectManager = ProjectManager::Instance();
        if (projectManager.IsProjectLoaded()) {
            std::string assetsDir = projectManager.GetAssetsDirectory();
            if (!std::filesystem::exists(assetsDir)) {
                std::filesystem::create_directories(assetsDir);
            }
            RenderDirectoryTree(assetsDir);
        } else {
            ImGui::Text("No project loaded");
        }
        
        RenderAssetImportDialog();
    }
    ImGui::End();
}

void ProjectPanel::RenderDirectoryTree(const std::string& path) {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_directory()) {
                std::string dirName = entry.path().filename().string();
                if (ImGui::TreeNode(dirName.c_str())) {
                    RenderDirectoryTree(entry.path().string());
                    ImGui::TreePop();
                }
            } else {
                RenderFileItem(entry.path().filename().string(), entry.path().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& ex) {
        ImGui::Text("Error reading directory: %s", ex.what());
    }
}

void ProjectPanel::RenderFileItem(const std::string& filename, const std::string& fullPath) {
    if (ImGui::Selectable(filename.c_str())) {
        Logger::Info("Selected file: " + fullPath);
    }
    
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Delete Asset")) {
            auto& projectManager = ProjectManager::Instance();
            std::string assetsDir = projectManager.GetAssetsDirectory();
            std::string relativePath = std::filesystem::relative(fullPath, assetsDir).string();
            if (projectManager.DeleteAsset(relativePath)) {
                Logger::Info("Asset deleted: " + relativePath);
            }
        }
        ImGui::EndPopup();
    }
}

void ProjectPanel::OpenAssetImportDialog() {
    m_showImportDialog = true;
    strcpy(m_importSourcePath, "");
    strcpy(m_importDestinationPath, "");
}

void ProjectPanel::RenderAssetImportDialog() {
    if (!m_showImportDialog) return;
    
    if (ImGui::BeginPopupModal("Import Asset", &m_showImportDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Import Asset to Project");
        ImGui::Separator();
        
        ImGui::InputText("Source Path", m_importSourcePath, sizeof(m_importSourcePath));
        ImGui::SameLine();
        if (ImGui::Button("Browse...")) {
            Logger::Info("File browser requested");
        }
        
        ImGui::InputText("Destination", m_importDestinationPath, sizeof(m_importDestinationPath));
        
        ImGui::Separator();
        
        if (ImGui::Button("Import")) {
            if (strlen(m_importSourcePath) > 0 && strlen(m_importDestinationPath) > 0) {
                auto& projectManager = ProjectManager::Instance();
                if (projectManager.ImportAsset(m_importSourcePath, m_importDestinationPath)) {
                    Logger::Info("Asset imported successfully");
                    m_showImportDialog = false;
                } else {
                    Logger::Error("Failed to import asset");
                }
            } else {
                Logger::Warning("Please specify both source and destination paths");
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_showImportDialog = false;
        }
        
        ImGui::EndPopup();
    }
    
    if (m_showImportDialog && !ImGui::IsPopupOpen("Import Asset")) {
        ImGui::OpenPopup("Import Asset");
    }
}

}
