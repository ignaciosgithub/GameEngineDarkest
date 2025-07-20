#pragma once

#include "../../../Core/Math/Vector2.h"

namespace GameEngine {
    class RigidBody2D;
    class Collider2D;
    class CircleCollider2D;
    class BoxCollider2D;
    
    struct CollisionInfo2D {
        bool hasCollision = false;
        Vector2 contactPoint;
        Vector2 normal;
        float penetration = 0.0f;
        RigidBody2D* bodyA = nullptr;
        RigidBody2D* bodyB = nullptr;
    };
    
    class CollisionDetection2D {
    public:
        // Main collision detection entry point
        static bool CheckCollision(RigidBody2D* bodyA, RigidBody2D* bodyB, CollisionInfo2D& info);
        
        // Specific collision detection methods
        static bool CircleVsCircle(const CircleCollider2D* circleA, const RigidBody2D* bodyA,
                                  const CircleCollider2D* circleB, const RigidBody2D* bodyB,
                                  CollisionInfo2D& info);
        
        static bool BoxVsBox(const BoxCollider2D* boxA, const RigidBody2D* bodyA,
                            const BoxCollider2D* boxB, const RigidBody2D* bodyB,
                            CollisionInfo2D& info);
        
        static bool CircleVsBox(const CircleCollider2D* circle, const RigidBody2D* circleBody,
                               const BoxCollider2D* box, const RigidBody2D* boxBody,
                               CollisionInfo2D& info);
        
        // AABB (Axis-Aligned Bounding Box) collision detection
        static bool AABBVsAABB(const Vector2& minA, const Vector2& maxA,
                              const Vector2& minB, const Vector2& maxB);
        
        // SAT (Separating Axis Theorem) for oriented boxes
        static bool SATBoxVsBox(const BoxCollider2D* boxA, const RigidBody2D* bodyA,
                               const BoxCollider2D* boxB, const RigidBody2D* bodyB,
                               CollisionInfo2D& info);
        
        // Point-in-shape tests
        static bool PointInCircle(const Vector2& point, const Vector2& center, float radius);
        static bool PointInBox(const Vector2& point, const BoxCollider2D* box, const RigidBody2D* body);
        
        // Distance calculations
        static float DistancePointToLine(const Vector2& point, const Vector2& lineStart, const Vector2& lineEnd);
        static Vector2 ClosestPointOnLine(const Vector2& point, const Vector2& lineStart, const Vector2& lineEnd);
        
        // Utility functions
        static Vector2 GetContactPoint(const CollisionInfo2D& info);
        static void ResolveCollision(RigidBody2D* bodyA, RigidBody2D* bodyB, const CollisionInfo2D& info);
        
    private:
        // Helper functions for SAT
        static void ProjectBoxOntoAxis(const BoxCollider2D* box, const RigidBody2D* body,
                                      const Vector2& axis, float& min, float& max);
        static bool OverlapOnAxis(float minA, float maxA, float minB, float maxB, float& overlap);
        static Vector2 GetBoxAxis(const BoxCollider2D* box, const RigidBody2D* body, int axisIndex);
    };
}
