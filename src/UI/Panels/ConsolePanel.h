#pragma once

#include "UIPanel.h"
#include <string>
#include <vector>

namespace GameEngine {
    struct LogEntry {
        std::string message;
        std::string level;
        std::string timestamp;
    };
    
    class ConsolePanel : public UIPanel {
    public:
        ConsolePanel();
        ~ConsolePanel() override;
        
        void Update(World* world, float deltaTime) override;
        void AddLogEntry(const std::string& message, const std::string& level);
        
    private:
        std::vector<LogEntry> m_logEntries;
        bool m_autoScroll = true;
        bool m_showInfo = true;
        bool m_showWarning = true;
        bool m_showError = true;
    };
}
