#pragma once

#include "../../Core/Math/Vector3.h"
#include "../../Core/Math/Matrix4.h"
#include "../../Core/Math/Quaternion.h"

namespace GameEngine {
    class RigidBody;
    class Octree;
    struct CollisionInfo;
    
    class CollisionDetection {
    public:
        static bool CheckCollision(RigidBody* bodyA, RigidBody* bodyB);
        static bool CheckCollision(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info);
        static bool CheckCollision(RigidBody* bodyA, RigidBody* bodyB, Octree* octree);
        static bool CheckCollision(RigidBody* bodyA, RigidBody* bodyB, Octree* octree, CollisionInfo& info);
        
        static bool SphereVsSphere(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info);
        static bool BoxVsBox(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info);
        static bool SphereVsBox(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info);
        
        // ConvexHull and TriangleMesh collision methods
        static bool ConvexHullVsConvexHull(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info);
        static bool TriangleMeshVsTriangleMesh(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info);
        static bool ConvexHullVsTriangleMesh(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info);
        static bool SphereVsConvexHull(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info);
        static bool BoxVsConvexHull(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info);
        
        // Helper methods for coordinate transformation
        static Vector3 TransformPoint(const Vector3& localPoint, const Vector3& position, const Quaternion& rotation, const Vector3& scale);
        static float TransformRadius(float localRadius, const Vector3& scale);
        static Vector3 TransformHalfExtents(const Vector3& localHalfExtents, const Vector3& scale);
        static Matrix4 GetOrientationMatrix(const Quaternion& rotation);
        
    public:
        static void ResolveCollision(RigidBody* bodyA, RigidBody* bodyB, const CollisionInfo& info);
    };
    
    struct CollisionInfo {
        bool hasCollision = false;
        Vector3 contactPoint;
        Vector3 normal;
        float penetration = 0.0f;
        RigidBody* bodyA = nullptr;
        RigidBody* bodyB = nullptr;
    };
}
