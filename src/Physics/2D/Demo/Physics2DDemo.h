#pragma once

#include "../../../Core/Math/Vector2.h"
#include <vector>
#include <memory>

namespace GameEngine {
    class PhysicsWorld2D;
    class RigidBody2D;
    
    class Physics2DDemo {
    public:
        Physics2DDemo();
        ~Physics2DDemo();
        
        void Initialize();
        void Shutdown();
        void Update(float deltaTime);
        
        // Demo scene creation
        void CreateBouncingBallsScene();
        void CreateStackingBoxesScene();
        void CreateMixedShapesScene();
        
        // Demo controls
        void SwitchToScene(int sceneIndex);
        void ResetCurrentScene();
        
        // Debug information
        int GetActiveBodyCount() const;
        int GetCollisionCount() const;
        
    private:
        std::unique_ptr<PhysicsWorld2D> m_physicsWorld;
        std::vector<std::unique_ptr<RigidBody2D>> m_rigidBodies;
        int m_currentScene = 0;
        bool m_initialized = false;
        
        void ClearScene();
        void CreateGround();
        void CreateWalls();
        RigidBody2D* CreateCircle(const Vector2& position, float radius, bool isDynamic = true);
        RigidBody2D* CreateBox(const Vector2& position, const Vector2& size, bool isDynamic = true);
    };
}
