#pragma once

struct GLFWwindow;

namespace GameEngine {
    class ImGuiRenderer {
    public:
        ImGuiRenderer();
        ~ImGuiRenderer();
        
        bool Initialize(GLFWwindow* window);
        void Shutdown();
        
        void BeginFrame();
        void EndFrame();
        
        void Render();
        
    private:
        bool m_initialized = false;
    };
}
