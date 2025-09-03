#include "ImGuiRenderer.h"
#include "../Core/Logging/Logger.h"
#include <imgui.h>
#if defined(_WIN32)
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#else
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#endif
#include <GLFW/glfw3.h>

namespace GameEngine {

ImGuiRenderer::ImGuiRenderer() = default;

ImGuiRenderer::~ImGuiRenderer() {
    Shutdown();
}

bool ImGuiRenderer::Initialize(GLFWwindow* window) {
    if (m_initialized) {
        Logger::Warning("ImGui Renderer already initialized");
        return true;
    }
    
    if (!window) {
        Logger::Error("Cannot initialize ImGui with null window");
        return false;
    }
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
#ifdef IMGUI_HAS_DOCK
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    
    ImGui_ImplOpenGL3_Init("#version 330");
    
    m_initialized = true;
    Logger::Info("ImGui Renderer initialized successfully");
    return true;
}

void ImGuiRenderer::Shutdown() {
    if (m_initialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        m_initialized = false;
        Logger::Info("ImGui Renderer shutdown");
    }
}

void ImGuiRenderer::BeginFrame() {
    if (!m_initialized) return;
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiRenderer::EndFrame() {
    if (!m_initialized) return;
    
    ImGui::Render();
}

void ImGuiRenderer::Render() {
    if (!m_initialized) return;
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

}
