#pragma once

#include "../../Core/Math/Vector3.h"

namespace GameEngine {
    class RigidBody;
    struct CollisionInfo;
    
    struct ContinuousCollisionInfo {
        bool hasCollision = false;
        float timeOfImpact = 1.0f; // Time from 0 to 1 when collision occurs
        Vector3 contactPoint;
        Vector3 normal;
        float penetration = 0.0f;
    };
    
    class ContinuousCollisionDetection {
    public:
        // Continuous collision detection using swept volumes
        static bool CheckContinuousCollision(RigidBody* bodyA, RigidBody* bodyB, 
                                           float deltaTime, ContinuousCollisionInfo& info);
        
        // Sphere vs Sphere continuous collision
        static bool SphereSphereSwept(RigidBody* bodyA, RigidBody* bodyB, 
                                    float deltaTime, ContinuousCollisionInfo& info);
        
        // Box vs Box continuous collision (simplified)
        static bool BoxBoxSwept(RigidBody* bodyA, RigidBody* bodyB, 
                              float deltaTime, ContinuousCollisionInfo& info);
        
        // Ray casting for fast-moving objects
        static bool RaycastAgainstBody(const Vector3& rayStart, const Vector3& rayEnd, 
                                     RigidBody* body, ContinuousCollisionInfo& info);
        
        // Time of impact calculation
        static float CalculateTimeOfImpact(RigidBody* bodyA, RigidBody* bodyB, float deltaTime);
        
    private:
        // Helper functions
        static bool SolveQuadratic(float a, float b, float c, float& t1, float& t2);
        static Vector3 GetBodyVelocityAtTime(RigidBody* body, float time);
        static Vector3 GetBodyPositionAtTime(RigidBody* body, float time, float deltaTime);
    };
}
