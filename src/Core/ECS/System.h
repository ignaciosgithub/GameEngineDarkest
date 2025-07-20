#pragma once

namespace GameEngine {
    class World;
    
    class ISystem {
    public:
        virtual ~ISystem() = default;
        virtual void Update(World* world, float deltaTime) = 0;
        virtual void Initialize(World* /*world*/) {}
        virtual void Shutdown(World* /*world*/) {}
    };
    
    template<typename T>
    class System : public ISystem {
    public:
        void Update(World* world, float deltaTime) override {
            static_cast<T*>(this)->OnUpdate(world, deltaTime);
        }
        
        void Initialize(World* world) override {
            static_cast<T*>(this)->OnInitialize(world);
        }
        
        void Shutdown(World* world) override {
            static_cast<T*>(this)->OnShutdown(world);
        }
        
    protected:
        virtual void OnUpdate(World* /*world*/, float /*deltaTime*/) {}
        virtual void OnInitialize(World* /*world*/) {}
        virtual void OnShutdown(World* /*world*/) {}
    };
}
