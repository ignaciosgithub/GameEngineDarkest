#pragma once

#include "UIPanel.h"

namespace GameEngine {
    class PhysicsWorld;
    
    class WorldSettingsPanel : public UIPanel {
    public:
        WorldSettingsPanel();
        ~WorldSettingsPanel() override;
        
        void Update(World* world, float deltaTime) override;
        
        void SetPhysicsWorld(PhysicsWorld* physicsWorld) { m_physicsWorld = physicsWorld; }
        
    private:
        void DrawGravitySettings();
        void DrawPhysicsSettings();
        void DrawPerformanceSettings();
        
        PhysicsWorld* m_physicsWorld = nullptr;
    };
}
