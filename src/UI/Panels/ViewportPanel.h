#pragma once

#include "UIPanel.h"
#include <imgui.h>
#include <memory>
#include "../../Rendering/Core/FrameBuffer.h"

namespace GameEngine {
    class ViewportPanel : public UIPanel {
    public:
        ViewportPanel();
        ~ViewportPanel() override;
        
        void Update(World* world, float deltaTime) override;
        
        bool IsViewportFocused() const { return m_viewportFocused; }
        bool IsViewportHovered() const { return m_viewportHovered; }
        
    private:
        bool m_viewportFocused = false;
        bool m_viewportHovered = false;
        
        ImVec2 m_viewportSize = {0.0f, 0.0f};
        bool m_viewportResized = false;
        std::shared_ptr<class FrameBuffer> m_sceneFramebuffer = nullptr;
    };
}
