#include "InputThread.h"
#include "Input.h"
#include "../Logging/Logger.h"
#include <chrono>

namespace GameEngine {
    
    InputThread::InputThread() = default;
    
    InputThread::~InputThread() {
        Stop();
    }
    
    void InputThread::Initialize(InputManager* inputManager) {
        if (m_initialized) {
            Logger::Warning("InputThread already initialized");
            return;
        }
        
        if (!inputManager) {
            Logger::Error("InputThread requires valid InputManager pointer");
            return;
        }
        
        m_inputManager = inputManager;
        m_initialized = true;
        
        Logger::Info("InputThread initialized successfully");
    }
    
    void InputThread::Start() {
        if (!m_initialized) {
            Logger::Error("InputThread not initialized");
            return;
        }
        
        if (m_running.load()) {
            Logger::Warning("InputThread already running");
            return;
        }
        
        m_shouldStop.store(false);
        m_running.store(true);
        
        m_inputThread = std::thread(&InputThread::ProcessInputLoop, this);
        
        Logger::Info("InputThread started successfully");
    }
    
    void InputThread::Stop() {
        if (!m_running.load()) {
            return;
        }
        
        Logger::Info("Stopping InputThread...");
        
        m_shouldStop.store(true);
        m_queueCondition.notify_all();
        
        if (m_inputThread.joinable()) {
            m_inputThread.join();
        }
        
        m_running.store(false);
        
        std::lock_guard<std::mutex> lock(m_queueMutex);
        while (!m_inputQueue.empty()) {
            m_inputQueue.pop();
        }
        
        Logger::Info("InputThread stopped successfully");
    }
    
    void InputThread::QueueKeyEvent(int key, int scancode, int action, int mods) {
        if (!m_running.load()) return;
        
        std::lock_guard<std::mutex> lock(m_queueMutex);
        
        if (m_inputQueue.size() >= MAX_QUEUE_SIZE) {
            Logger::Warning("Input queue full, dropping key event");
            return;
        }
        
        InputEvent event(InputEvent::KeyEvent);
        event.key = key;
        event.scancode = scancode;
        event.action = action;
        event.mods = mods;
        
        m_inputQueue.push(event);
        m_queueCondition.notify_one();
    }
    
    void InputThread::QueueMouseButtonEvent(int button, int action, int mods) {
        if (!m_running.load()) return;
        
        std::lock_guard<std::mutex> lock(m_queueMutex);
        
        if (m_inputQueue.size() >= MAX_QUEUE_SIZE) {
            Logger::Warning("Input queue full, dropping mouse button event");
            return;
        }
        
        InputEvent event(InputEvent::MouseButtonEvent);
        event.button = button;
        event.action = action;
        event.mods = mods;
        
        m_inputQueue.push(event);
        m_queueCondition.notify_one();
    }
    
    void InputThread::QueueMouseMoveEvent(double xpos, double ypos) {
        if (!m_running.load()) return;
        
        std::lock_guard<std::mutex> lock(m_queueMutex);
        
        if (m_inputQueue.size() >= MAX_QUEUE_SIZE) {
            Logger::Warning("Input queue full, dropping mouse move event");
            return;
        }
        
        InputEvent event(InputEvent::MouseMoveEvent);
        event.xpos = xpos;
        event.ypos = ypos;
        
        m_inputQueue.push(event);
        m_queueCondition.notify_one();
    }
    
    void InputThread::ProcessInputLoop() {
        Logger::Debug("InputThread processing loop started");
        
        while (!m_shouldStop.load()) {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            
            m_queueCondition.wait(lock, [this] {
                return !m_inputQueue.empty() || m_shouldStop.load();
            });
            
            while (!m_inputQueue.empty() && !m_shouldStop.load()) {
                InputEvent event = m_inputQueue.front();
                m_inputQueue.pop();
                
                lock.unlock();
                ProcessEvent(event);
                lock.lock();
            }
        }
        
        Logger::Debug("InputThread processing loop ended");
    }
    
    void InputThread::ProcessEvent(const InputEvent& event) {
        if (!m_inputManager) return;
        
        switch (event.type) {
            case InputEvent::KeyEvent:
                m_inputManager->OnKeyEventThreaded(event.key, event.scancode, event.action, event.mods);
                break;
                
            case InputEvent::MouseButtonEvent:
                m_inputManager->OnMouseButtonEventThreaded(event.button, event.action, event.mods);
                break;
                
            case InputEvent::MouseMoveEvent:
                m_inputManager->OnMouseMoveEventThreaded(event.xpos, event.ypos);
                break;
        }
    }
}
