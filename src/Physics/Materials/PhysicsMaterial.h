#pragma once

#include <string>

namespace GameEngine {
    class PhysicsMaterial {
    public:
        PhysicsMaterial(const std::string& name = "Default");
        ~PhysicsMaterial() = default;
        
        // Material properties
        float GetDynamicFriction() const { return m_dynamicFriction; }
        void SetDynamicFriction(float friction) { m_dynamicFriction = friction; }
        
        float GetStaticFriction() const { return m_staticFriction; }
        void SetStaticFriction(float friction) { m_staticFriction = friction; }
        
        float GetRestitution() const { return m_restitution; }
        void SetRestitution(float restitution) { m_restitution = restitution; }
        
        float GetDensity() const { return m_density; }
        void SetDensity(float density) { m_density = density; }
        
        // Friction combination modes
        enum class FrictionCombine {
            Average,
            Minimum,
            Maximum,
            Multiply
        };
        
        enum class RestitutionCombine {
            Average,
            Minimum,
            Maximum,
            Multiply
        };
        
        FrictionCombine GetFrictionCombine() const { return m_frictionCombine; }
        void SetFrictionCombine(FrictionCombine combine) { m_frictionCombine = combine; }
        
        RestitutionCombine GetRestitutionCombine() const { return m_restitutionCombine; }
        void SetRestitutionCombine(RestitutionCombine combine) { m_restitutionCombine = combine; }
        
        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }
        
        // Static material combination functions
        static float CombineFriction(float friction1, float friction2, FrictionCombine mode);
        static float CombineRestitution(float restitution1, float restitution2, RestitutionCombine mode);
        
        // Predefined materials
        static PhysicsMaterial* GetDefault();
        static PhysicsMaterial* GetBouncy();
        static PhysicsMaterial* GetIce();
        static PhysicsMaterial* GetRubber();
        static PhysicsMaterial* GetMetal();
        
    private:
        std::string m_name;
        float m_dynamicFriction = 0.6f;
        float m_staticFriction = 0.6f;
        float m_restitution = 0.0f;
        float m_density = 1.0f;
        
        FrictionCombine m_frictionCombine = FrictionCombine::Average;
        RestitutionCombine m_restitutionCombine = RestitutionCombine::Average;
    };
}
