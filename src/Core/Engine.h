#pragma once

#include <memory>
#include <string>

namespace GameEngine {
    class World;
    class Window;
    class Renderer;
    class RenderManager;
    class InputManager;
    class PhysicsWorld;
    class TestSceneManager;
    class EngineUI;
    class PlayModeManager;
    class SelectionManager;

    class Engine {
    public:
        Engine();
        ~Engine();

        bool Initialize(const std::string& title = "Game Engine", int width = 1280, int height = 720);
        void Run();
        void Shutdown();

        World* GetWorld() const { return m_world.get(); }
        Window* GetWindow() const { return m_window.get(); }
        Renderer* GetRenderer() const { return m_renderer.get(); }
        RenderManager* GetRenderManager() const { return m_renderManager.get(); }
        InputManager* GetInputManager() const { return m_inputManager.get(); }
        PhysicsWorld* GetPhysicsWorld() const { return m_physicsWorld.get(); }
        TestSceneManager* GetTestSceneManager() const { return m_testSceneManager.get(); }
        EngineUI* GetEngineUI() const { return m_engineUI.get(); }
        PlayModeManager* GetPlayModeManager() const { return m_playModeManager.get(); }

        bool IsRunning() const { return m_isRunning; }
        void Stop() { m_isRunning = false; }

    private:
        void Update(float deltaTime);
        void Render();
        void CreateDemoScene();

        std::unique_ptr<World> m_world;
        std::unique_ptr<Window> m_window;
        std::unique_ptr<Renderer> m_renderer;
        std::unique_ptr<RenderManager> m_renderManager;
        std::unique_ptr<InputManager> m_inputManager;
        std::unique_ptr<PhysicsWorld> m_physicsWorld;
        std::unique_ptr<TestSceneManager> m_testSceneManager;
        std::unique_ptr<EngineUI> m_engineUI;
        std::unique_ptr<PlayModeManager> m_playModeManager;
        std::unique_ptr<SelectionManager> m_selectionManager;

        bool m_isRunning = false;
    };
}
