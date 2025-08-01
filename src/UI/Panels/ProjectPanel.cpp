#include "ProjectPanel.h"
#include "../../Core/Logging/Logger.h"
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
        
        if (!std::filesystem::exists(m_currentPath)) {
            std::filesystem::create_directories(m_currentPath);
        }
        
        RenderDirectoryTree(m_currentPath);
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
}

}
