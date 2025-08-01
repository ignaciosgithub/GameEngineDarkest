#include "ImGuiRenderer.h"
#include "../Core/Logging/Logger.h"
#include <imgui.h>
// #include <imgui_impl_glfw.h>
// #include <imgui_impl_opengl3.h>
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
    
    ImGui::StyleColorsDark();
    
    m_initialized = true;
    Logger::Info("ImGui Renderer initialized successfully");
    return true;
}

void ImGuiRenderer::Shutdown() {
    if (m_initialized) {
        ImGui::DestroyContext();
        
        m_initialized = false;
        Logger::Info("ImGui Renderer shutdown");
    }
}

void ImGuiRenderer::BeginFrame() {
    if (!m_initialized) return;
    
    ImGui::NewFrame();
}

void ImGuiRenderer::EndFrame() {
    if (!m_initialized) return;
    
    ImGui::Render();
}

void ImGuiRenderer::Render() {
    EndFrame();
}

}
