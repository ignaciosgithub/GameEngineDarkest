#include "ProjectPanel.h"
#include "../../Core/Logging/Logger.h"
#include "../../Core/Project/ProjectManager.h"
#include <imgui.h>
#include <filesystem>
#ifdef NFD_AVAILABLE
#include <nfd.h>
#endif

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
#ifdef _WIN32
    strcpy_s(m_importSourcePath, sizeof(m_importSourcePath), "");
    strcpy_s(m_importDestinationPath, sizeof(m_importDestinationPath), "");
#else
    strcpy(m_importSourcePath, "");
    strcpy(m_importDestinationPath, "");
#endif
}

void ProjectPanel::RenderAssetImportDialog() {
    if (!m_showImportDialog) return;
    
    if (ImGui::BeginPopupModal("Import Asset", &m_showImportDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Import Asset to Project");
        ImGui::Separator();
        
        ImGui::InputText("Source Path", m_importSourcePath, sizeof(m_importSourcePath));
        ImGui::SameLine();
        if (ImGui::Button("Browse...")) {
#ifdef NFD_AVAILABLE
            nfdchar_t *outPath;
            nfdfilteritem_t filterItem[1] = { { "All Files", "*" } };
            nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
            
            if (result == NFD_OKAY) {
                strncpy(m_importSourcePath, outPath, sizeof(m_importSourcePath) - 1);
                m_importSourcePath[sizeof(m_importSourcePath) - 1] = '\0';
                Logger::Info("Selected file: " + std::string(outPath));
                NFD_FreePath(outPath);
            } else if (result == NFD_CANCEL) {
                Logger::Info("File dialog cancelled");
            } else {
                Logger::Error("File dialog error: " + std::string(NFD_GetError()));
            }
#else
            Logger::Info("File browser not available - nativefiledialog-extended not found during build");
#endif
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
