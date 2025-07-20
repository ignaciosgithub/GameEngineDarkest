#pragma once

namespace GameEngine {
    class World;
    
    class UIPanel {
    public:
        virtual ~UIPanel() = default;
        
        virtual void Update(World* world, float deltaTime) = 0;
        
        bool& IsVisible() { return m_visible; }
        const bool& IsVisible() const { return m_visible; }
        
    protected:
        bool m_visible = true;
    };
}
