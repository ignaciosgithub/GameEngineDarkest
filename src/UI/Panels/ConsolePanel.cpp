#include "ConsolePanel.h"
#include <imgui.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace GameEngine {

ConsolePanel::ConsolePanel() = default;

ConsolePanel::~ConsolePanel() = default;

void ConsolePanel::Update(World* /*world*/, float /*deltaTime*/) {
    if (!m_visible) return;
    
    if (ImGui::Begin("Console", &m_visible)) {
        ImGui::Checkbox("Info", &m_showInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warning", &m_showWarning);
        ImGui::SameLine();
        ImGui::Checkbox("Error", &m_showError);
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            m_logEntries.clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto Scroll", &m_autoScroll);
        
        ImGui::Separator();
        
        if (ImGui::BeginChild("LogEntries")) {
            for (const auto& entry : m_logEntries) {
                bool show = false;
                ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                
                if (entry.level == "INFO" && m_showInfo) {
                    show = true;
                    color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
                } else if (entry.level == "WARNING" && m_showWarning) {
                    show = true;
                    color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                } else if (entry.level == "ERROR" && m_showError) {
                    show = true;
                    color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                }
                
                if (show) {
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::Text("[%s] %s: %s", entry.timestamp.c_str(), entry.level.c_str(), entry.message.c_str());
                    ImGui::PopStyleColor();
                }
            }
            
            if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void ConsolePanel::AddLogEntry(const std::string& message, const std::string& level) {
    LogEntry entry;
    entry.message = message;
    entry.level = level;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    
#ifdef _WIN32
    struct tm timeinfo;
    localtime_s(&timeinfo, &time_t);
    ss << std::put_time(&timeinfo, "%H:%M:%S");
#else
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
#endif
    
    entry.timestamp = ss.str();
    
    m_logEntries.push_back(entry);
    
    if (m_logEntries.size() > 1000) {
        m_logEntries.erase(m_logEntries.begin());
    }
}

}
