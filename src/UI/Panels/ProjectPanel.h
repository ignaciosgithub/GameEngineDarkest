#pragma once

#include "UIPanel.h"
#include <string>
#include <vector>

namespace GameEngine {
    class ProjectPanel : public UIPanel {
    public:
        ProjectPanel();
        ~ProjectPanel() override;
        
        void Update(World* world, float deltaTime) override;
        
    private:
        void RenderDirectoryTree(const std::string& path);
        void RenderFileItem(const std::string& filename, const std::string& fullPath);
        void OpenAssetImportDialog();
        void RenderAssetImportDialog();
        
        std::string m_currentPath = "assets/";
        std::vector<std::string> m_selectedFiles;
        bool m_showImportDialog = false;
        char m_importSourcePath[512] = "";
        char m_importDestinationPath[256] = "";
    };
}
