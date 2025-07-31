#include "EngineUI.h"
#include "ImGuiRenderer.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ViewportPanel.h"
#include "../Core/Logging/Logger.h"
#include "../Core/Editor/PlayModeManager.h"
#include <imgui.h>

namespace GameEngine {

EngineUI::EngineUI() = default;

EngineUI::~EngineUI() {
    Shutdown();
}

bool EngineUI::Initialize(GLFWwindow* window) {
    m_imguiRenderer = std::make_unique<ImGuiRenderer>();
    if (!m_imguiRenderer->Initialize(window)) {
        Logger::Error("Failed to initialize ImGui renderer");
        return false;
    }
    
    m_panels.push_back(std::make_unique<SceneHierarchyPanel>());
    m_panels.push_back(std::make_unique<InspectorPanel>());
    m_panels.push_back(std::make_unique<ViewportPanel>());
    
    Logger::Info("Engine UI initialized successfully");
    return true;
}

void EngineUI::Shutdown() {
    m_panels.clear();
    
    if (m_imguiRenderer) {
        m_imguiRenderer->Shutdown();
        m_imguiRenderer.reset();
    }
    
    Logger::Info("Engine UI shutdown");
}

void EngineUI::Update(World* world, float deltaTime) {
    m_world = world;
    
    if (!m_imguiRenderer) return;
    
    m_imguiRenderer->BeginFrame();
    
    RenderDockSpace();
    RenderMainMenuBar();
    
    for (auto& panel : m_panels) {
        panel->Update(world, deltaTime);
    }
    
    if (m_showDemoWindow) {
        Logger::Debug("Demo window would be shown");
    }
    
    if (m_showMetricsWindow) {
        Logger::Debug("Metrics window would be shown");
    }
}

void EngineUI::Render() {
    if (m_imguiRenderer) {
        m_imguiRenderer->Render();
    }
}

void EngineUI::RenderMainMenuBar() {
    if (!m_playModeManager) {
        Logger::Debug("Main menu bar rendering (simplified mode - no PlayModeManager)");
        return;
    }
    
    EditorMode currentMode = m_playModeManager->GetCurrentMode();
    bool cursorLocked = m_playModeManager->IsCursorLocked();
    
    std::string modeStr = (currentMode == EditorMode::Edit ? "Edit" : 
                           currentMode == EditorMode::Play ? "Play" : "Paused");
    std::string cursorStr = (cursorLocked ? "Locked" : "Unlocked");
    
    Logger::Debug("Main menu bar - Mode: " + modeStr + ", Cursor: " + cursorStr + " (simplified mode)");
}

void EngineUI::RenderDockSpace() {
    static bool dockspaceOpen = true;
    
    if (dockspaceOpen) {
        Logger::Debug("DockSpace rendering (simplified mode)");
    }
}

}
