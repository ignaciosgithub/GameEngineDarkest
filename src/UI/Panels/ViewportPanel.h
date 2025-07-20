#pragma once

#include "UIPanel.h"

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
    };
}
