#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <memory>

namespace GameEngine {
    
    struct InputEvent {
        enum Type {
            KeyEvent,
            MouseButtonEvent,
            MouseMoveEvent
        };
        
        Type type;
        
        // Key event data
        int key = 0;
        int scancode = 0;
        int action = 0;
        int mods = 0;
        
        // Mouse button event data
        int button = 0;
        
        // Mouse move event data
        double xpos = 0.0;
        double ypos = 0.0;
        
        InputEvent(Type t) : type(t) {}
    };
    
    class InputManager;
    
    class InputThread {
    public:
        InputThread();
        ~InputThread();
        
        void Initialize(InputManager* inputManager);
        void Start();
        void Stop();
        
        void QueueKeyEvent(int key, int scancode, int action, int mods);
        void QueueMouseButtonEvent(int button, int action, int mods);
        void QueueMouseMoveEvent(double xpos, double ypos);
        
        bool IsRunning() const { return m_running.load(); }
        
    private:
        void ProcessInputLoop();
        void ProcessEvent(const InputEvent& event);
        
        std::thread m_inputThread;
        std::atomic<bool> m_running{false};
        std::atomic<bool> m_shouldStop{false};
        
        std::mutex m_queueMutex;
        std::queue<InputEvent> m_inputQueue;
        std::condition_variable m_queueCondition;
        
        InputManager* m_inputManager = nullptr;
        bool m_initialized = false;
        
        static constexpr size_t MAX_QUEUE_SIZE = 1000;
    };
}
