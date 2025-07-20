#pragma once

#include "../../Core/Math/Vector3.h"

namespace GameEngine {
    class RigidBody;
    struct CollisionInfo;
    
    class CollisionDetection {
    public:
        static bool CheckCollision(RigidBody* bodyA, RigidBody* bodyB);
        
        static bool SphereVsSphere(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info);
        static bool BoxVsBox(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info);
        static bool SphereVsBox(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info);
        
    private:
        static void ResolveCollision(RigidBody* bodyA, RigidBody* bodyB, const CollisionInfo& info);
    };
    
    struct CollisionInfo {
        bool hasCollision = false;
        Vector3 contactPoint;
        Vector3 normal;
        float penetration = 0.0f;
    };
}
