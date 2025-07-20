#include "ViewportPanel.h"
#include "../../Core/Logging/Logger.h"
#include <imgui.h>

namespace GameEngine {

ViewportPanel::ViewportPanel() = default;

ViewportPanel::~ViewportPanel() = default;

void ViewportPanel::Update(World* world, float deltaTime) {
    if (!m_visible) return;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport", &m_visible);
    
    m_viewportFocused = ImGui::IsWindowFocused();
    m_viewportHovered = ImGui::IsWindowHovered();
    
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    
    ImGui::Text("3D Viewport");
    ImGui::Text("Size: %.0f x %.0f", viewportPanelSize.x, viewportPanelSize.y);
    
    
    if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = viewportPanelSize;
        
        drawList->AddRectFilled(
            canvasPos,
            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
            IM_COL32(50, 50, 50, 255)
        );
        
        for (float x = 0; x < canvasSize.x; x += 50.0f) {
            drawList->AddLine(
                ImVec2(canvasPos.x + x, canvasPos.y),
                ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y),
                IM_COL32(100, 100, 100, 255)
            );
        }
        
        for (float y = 0; y < canvasSize.y; y += 50.0f) {
            drawList->AddLine(
                ImVec2(canvasPos.x, canvasPos.y + y),
                ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y),
                IM_COL32(100, 100, 100, 255)
            );
        }
    }
    
    ImGui::End();
    ImGui::PopStyleVar();
}

}
