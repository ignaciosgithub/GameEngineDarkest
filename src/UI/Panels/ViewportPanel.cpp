#include "ViewportPanel.h"
#include "../../Core/Logging/Logger.h"
#include <imgui.h>

namespace GameEngine {

ViewportPanel::ViewportPanel() = default;

ViewportPanel::~ViewportPanel() = default;

void ViewportPanel::Update(World* /*world*/, float /*deltaTime*/) {
    if (!m_visible) return;
    
    if (ImGui::Begin("Viewport", &m_visible)) {
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        
        if (viewportPanelSize.x != m_viewportSize.x || viewportPanelSize.y != m_viewportSize.y) {
            m_viewportSize = viewportPanelSize;
            m_viewportResized = true;
        }
        
        if (m_sceneFramebuffer) {
            auto colorTexture = m_sceneFramebuffer->GetColorTexture(0);
            if (colorTexture) {
                ImGui::Image((ImTextureID)(intptr_t)colorTexture->GetID(), 
                            ImVec2(m_viewportSize.x, m_viewportSize.y), 
                            ImVec2(0, 1), ImVec2(1, 0));
            } else {
                ImGui::Text("Color texture not available");
            }
        } else {
            ImGui::Text("Scene rendering not available");
            ImGui::Text("Viewport Size: %.0f x %.0f", m_viewportSize.x, m_viewportSize.y);
        }
        
        m_viewportFocused = ImGui::IsWindowFocused();
        m_viewportHovered = ImGui::IsWindowHovered();
    }
    ImGui::End();
}

}
