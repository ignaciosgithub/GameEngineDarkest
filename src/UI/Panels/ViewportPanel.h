#pragma once

#include "UIPanel.h"
#include <imgui.h>
#include <memory>
#include "../../Rendering/Core/FrameBuffer.h"
#include "../../Core/Math/Vector2.h"

namespace GameEngine {
    class PlayModeManager;
    class SelectionManager;
    
    class ViewportPanel : public UIPanel {
    public:
        ViewportPanel();
        ~ViewportPanel() override;
        
        void Update(World* world, float deltaTime) override;
        
        bool IsViewportFocused() const { return m_viewportFocused; }
        bool IsViewportHovered() const { return m_viewportHovered; }
        
        void SetFramebuffer(std::shared_ptr<FrameBuffer> framebuffer) { m_sceneFramebuffer = framebuffer; }
        void SetPlayModeManager(PlayModeManager* playModeManager) { m_playModeManager = playModeManager; }
        void SetSelectionManager(SelectionManager* selectionManager) { m_selectionManager = selectionManager; }
        
    private:
        void HandleViewportClick(const Vector2& relativePos, World* world);
        
        bool m_viewportFocused = false;
        bool m_viewportHovered = false;
        
        ImVec2 m_viewportSize = {0.0f, 0.0f};
        bool m_viewportResized = false;
        std::shared_ptr<class FrameBuffer> m_sceneFramebuffer = nullptr;
        
        PlayModeManager* m_playModeManager = nullptr;
        SelectionManager* m_selectionManager = nullptr;
    };
}
