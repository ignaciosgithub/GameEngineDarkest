#pragma once

#include <memory>
#include <vector>

struct GLFWwindow;

namespace GameEngine {
    class World;
    class ImGuiRenderer;
    class UIPanel;
    class PlayModeManager;
    enum class EditorMode;
    
    class EngineUI {
    public:
        EngineUI();
        ~EngineUI();
        
        bool Initialize(GLFWwindow* window);
        void Shutdown();
        
        void Update(World* world, float deltaTime);
        void Render();
        
        void SetWorld(World* world) { m_world = world; }
        void SetPlayModeManager(PlayModeManager* playModeManager) { m_playModeManager = playModeManager; }
        
        class ViewportPanel* GetViewportPanel() const;
        
    private:
        void RenderMainMenuBar();
        void RenderDockSpace();
        void CreateNewScene();
        void SaveCurrentScene(const std::string& sceneName);
        void SaveSceneAs(const std::string& sceneName);
        
        std::unique_ptr<ImGuiRenderer> m_imguiRenderer;
        std::vector<std::unique_ptr<UIPanel>> m_panels;
        
        World* m_world = nullptr;
        PlayModeManager* m_playModeManager = nullptr;
        bool m_showDemoWindow = false;
        bool m_showMetricsWindow = false;
    };
}
