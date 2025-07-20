#pragma once

#include <memory>
#include <vector>

namespace GameEngine {
    class World;
    class ImGuiRenderer;
    class UIPanel;
    
    class EngineUI {
    public:
        EngineUI();
        ~EngineUI();
        
        bool Initialize(struct GLFWwindow* window);
        void Shutdown();
        
        void Update(World* world, float deltaTime);
        void Render();
        
        void SetWorld(World* world) { m_world = world; }
        
    private:
        void RenderMainMenuBar();
        void RenderDockSpace();
        
        std::unique_ptr<ImGuiRenderer> m_imguiRenderer;
        std::vector<std::unique_ptr<UIPanel>> m_panels;
        
        World* m_world = nullptr;
        bool m_showDemoWindow = false;
        bool m_showMetricsWindow = false;
    };
}
