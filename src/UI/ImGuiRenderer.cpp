#include "ImGuiRenderer.h"
#include "../Core/Logging/Logger.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
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
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        Logger::Error("Failed to initialize ImGui GLFW backend");
        return false;
    }
    
    if (!ImGui_ImplOpenGL3_Init("#version 450")) {
        Logger::Error("Failed to initialize ImGui OpenGL3 backend");
        ImGui_ImplGlfw_Shutdown();
        return false;
    }
    
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
    
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void ImGuiRenderer::Render() {
    EndFrame();
}

}
