#include "ViewportPanel.h"
#include "../../Core/Logging/Logger.h"
#include <imgui.h>

namespace GameEngine {

ViewportPanel::ViewportPanel() = default;

ViewportPanel::~ViewportPanel() = default;

void ViewportPanel::Update(World* /*world*/, float /*deltaTime*/) {
    if (!m_visible) return;
    
    m_viewportFocused = true;
    m_viewportHovered = true;
    
    Logger::Debug("Viewport panel updated - 3D Viewport (simplified mode)");
}

}
