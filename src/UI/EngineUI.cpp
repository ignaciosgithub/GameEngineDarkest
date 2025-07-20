#include "EngineUI.h"
#include "ImGuiRenderer.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ViewportPanel.h"
#include "../Core/Logging/Logger.h"
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
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }
    
    if (m_showMetricsWindow) {
        ImGui::ShowMetricsWindow(&m_showMetricsWindow);
    }
}

void EngineUI::Render() {
    if (m_imguiRenderer) {
        m_imguiRenderer->Render();
    }
}

void EngineUI::RenderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) {
                Logger::Info("New Scene requested");
            }
            if (ImGui::MenuItem("Open Scene")) {
                Logger::Info("Open Scene requested");
            }
            if (ImGui::MenuItem("Save Scene")) {
                Logger::Info("Save Scene requested");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                Logger::Info("Exit requested");
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo")) {
                Logger::Info("Undo requested");
            }
            if (ImGui::MenuItem("Redo")) {
                Logger::Info("Redo requested");
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Scene Hierarchy", nullptr, &m_panels[0]->IsVisible());
            ImGui::MenuItem("Inspector", nullptr, &m_panels[1]->IsVisible());
            ImGui::MenuItem("Viewport", nullptr, &m_panels[2]->IsVisible());
            ImGui::Separator();
            ImGui::MenuItem("Demo Window", nullptr, &m_showDemoWindow);
            ImGui::MenuItem("Metrics", nullptr, &m_showMetricsWindow);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                Logger::Info("About dialog requested");
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void EngineUI::RenderDockSpace() {
    static bool dockspaceOpen = true;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();
    
    if (opt_fullscreen)
        ImGui::PopStyleVar(2);
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    
    ImGui::End();
}

}
