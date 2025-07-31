#pragma once

#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../ECS/Entity.h"
#include "../../Physics/PhysicsWorld.h"
#include "../../Physics/2D/PhysicsWorld2D.h"
#include "../../Physics/RigidBody/RigidBody.h"
#include "../../Physics/2D/RigidBody2D.h"
#include <vector>

namespace GameEngine {

class World;
class Entity;

struct RayHit3D {
    bool hit = false;
    Vector3 point;
    Vector3 normal;
    float distance = 0.0f;
    RigidBody* rigidBody = nullptr;
    Entity entity; // Default constructor initializes with INVALID_ENTITY
};

struct RayHit2D {
    bool hit = false;
    Vector2 point;
    Vector2 normal;
    float distance = 0.0f;
    RigidBody2D* rigidBody2D = nullptr;
    Entity entity; // Default constructor initializes with INVALID_ENTITY
};

struct Ray3D {
    Vector3 origin;
    Vector3 direction;
    float maxDistance = 1000.0f;
    
    Ray3D(const Vector3& origin, const Vector3& direction, float maxDistance = 1000.0f)
        : origin(origin), direction(direction.Normalized()), maxDistance(maxDistance) {}
};

struct Ray2D {
    Vector2 origin;
    Vector2 direction;
    float maxDistance = 1000.0f;
    
    Ray2D(const Vector2& origin, const Vector2& direction, float maxDistance = 1000.0f)
        : origin(origin), direction(direction.Normalized()), maxDistance(maxDistance) {}
};

class RayCaster {
public:
    RayCaster();
    ~RayCaster();
    
    // Initialize with physics worlds
    void Initialize(PhysicsWorld* physicsWorld3D, PhysicsWorld2D* physicsWorld2D);
    void Shutdown();
    
    // 3D Ray casting
    bool Raycast3D(const Ray3D& ray, RayHit3D& hit);
    bool Raycast3D(const Vector3& origin, const Vector3& direction, RayHit3D& hit, float maxDistance = 1000.0f);
    
    // 3D Ray casting with multiple hits
    std::vector<RayHit3D> RaycastAll3D(const Ray3D& ray);
    std::vector<RayHit3D> RaycastAll3D(const Vector3& origin, const Vector3& direction, float maxDistance = 1000.0f);
    
    // 2D Ray casting
    bool Raycast2D(const Ray2D& ray, RayHit2D& hit);
    bool Raycast2D(const Vector2& origin, const Vector2& direction, RayHit2D& hit, float maxDistance = 1000.0f);
    
    // 2D Ray casting with multiple hits
    std::vector<RayHit2D> RaycastAll2D(const Ray2D& ray);
    std::vector<RayHit2D> RaycastAll2D(const Vector2& origin, const Vector2& direction, float maxDistance = 1000.0f);
    
    // Screen to world ray casting (for editor picking)
    Ray3D ScreenPointToRay(const Vector2& screenPoint, const class CameraComponent* camera, int screenWidth, int screenHeight);
    bool ScreenPointRaycast(const Vector2& screenPoint, const class CameraComponent* camera, int screenWidth, int screenHeight, RayHit3D& hit);
    
    // Utility methods
    bool IsPointInSphere(const Vector3& point, const Vector3& sphereCenter, float sphereRadius);
    bool IsPointInBox(const Vector3& point, const Vector3& boxCenter, const Vector3& boxSize);
    bool IsPointInCircle(const Vector2& point, const Vector2& circleCenter, float circleRadius);
    bool IsPointInRect(const Vector2& point, const Vector2& rectCenter, const Vector2& rectSize);
    
    // Layer filtering (for selective ray casting)
    void SetLayerMask(uint32_t layerMask) { m_layerMask = layerMask; }
    uint32_t GetLayerMask() const { return m_layerMask; }
    
    // Settings
    void SetMaxRaycastHits(int maxHits) { m_maxRaycastHits = maxHits; }
    int GetMaxRaycastHits() const { return m_maxRaycastHits; }
    
private:
    PhysicsWorld* m_physicsWorld3D = nullptr;
    PhysicsWorld2D* m_physicsWorld2D = nullptr;
    uint32_t m_layerMask = 0xFFFFFFFF; // All layers by default
    int m_maxRaycastHits = 32;
    
    // Internal ray casting implementations
    bool RaycastAgainstRigidBody3D(const Ray3D& ray, RigidBody* body, RayHit3D& hit);
    bool RaycastAgainstRigidBody2D(const Ray2D& ray, RigidBody2D* body, RayHit2D& hit);
    
    // Helper methods
    Vector3 CalculateRayPoint3D(const Ray3D& ray, float distance);
    Vector2 CalculateRayPoint2D(const Ray2D& ray, float distance);
    bool SolveQuadratic(float a, float b, float c, float& t1, float& t2);
};

}
