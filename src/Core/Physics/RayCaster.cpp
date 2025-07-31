#include "RayCaster.h"
#include "../Components/CameraComponent.h"
#include "../Logging/Logger.h"
#include "../../Physics/Collision/ContinuousCollisionDetection.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {

RayCaster::RayCaster() {
    Logger::Debug("RayCaster created");
}

RayCaster::~RayCaster() {
    Shutdown();
    Logger::Debug("RayCaster destroyed");
}

void RayCaster::Initialize(PhysicsWorld* physicsWorld3D, PhysicsWorld2D* physicsWorld2D) {
    m_physicsWorld3D = physicsWorld3D;
    m_physicsWorld2D = physicsWorld2D;
    
    Logger::Info("RayCaster initialized with physics worlds");
}

void RayCaster::Shutdown() {
    m_physicsWorld3D = nullptr;
    m_physicsWorld2D = nullptr;
    Logger::Debug("RayCaster shutdown complete");
}

bool RayCaster::Raycast3D(const Ray3D& ray, RayHit3D& hit) {
    if (!m_physicsWorld3D) {
        Logger::Warning("RayCaster::Raycast3D - PhysicsWorld3D is null");
        return false;
    }
    
    hit.hit = false;
    hit.distance = ray.maxDistance;
    
    Logger::Debug("Performing 3D raycast from (" + std::to_string(ray.origin.x) + ", " + 
                 std::to_string(ray.origin.y) + ", " + std::to_string(ray.origin.z) + 
                 ") in direction (" + std::to_string(ray.direction.x) + ", " + 
                 std::to_string(ray.direction.y) + ", " + std::to_string(ray.direction.z) + ")");
    
    
    return hit.hit;
}

bool RayCaster::Raycast3D(const Vector3& origin, const Vector3& direction, RayHit3D& hit, float maxDistance) {
    Ray3D ray(origin, direction, maxDistance);
    return Raycast3D(ray, hit);
}

std::vector<RayHit3D> RayCaster::RaycastAll3D(const Ray3D& ray) {
    std::vector<RayHit3D> hits;
    hits.reserve(m_maxRaycastHits);
    
    if (!m_physicsWorld3D) {
        Logger::Warning("RayCaster::RaycastAll3D - PhysicsWorld3D is null");
        return hits;
    }
    
    RayHit3D hit;
    if (Raycast3D(ray, hit)) {
        hits.push_back(hit);
    }
    
    return hits;
}

std::vector<RayHit3D> RayCaster::RaycastAll3D(const Vector3& origin, const Vector3& direction, float maxDistance) {
    Ray3D ray(origin, direction, maxDistance);
    return RaycastAll3D(ray);
}

bool RayCaster::Raycast2D(const Ray2D& ray, RayHit2D& hit) {
    if (!m_physicsWorld2D) {
        Logger::Warning("RayCaster::Raycast2D - PhysicsWorld2D is null");
        return false;
    }
    
    hit.hit = false;
    hit.distance = ray.maxDistance;
    
    Vector2 rayEnd = ray.origin + ray.direction * ray.maxDistance;
    
    RigidBody2D* hitBody = nullptr;
    Vector2 hitPoint, hitNormal;
    
    if (m_physicsWorld2D->Raycast(ray.origin, rayEnd, hitBody, hitPoint, hitNormal)) {
        hit.hit = true;
        hit.point = hitPoint;
        hit.normal = hitNormal;
        hit.distance = (hitPoint - ray.origin).Length();
        hit.rigidBody2D = hitBody;
        
        Logger::Debug("2D Raycast hit at (" + std::to_string(hitPoint.x) + ", " + std::to_string(hitPoint.y) + ")");
        return true;
    }
    
    return false;
}

bool RayCaster::Raycast2D(const Vector2& origin, const Vector2& direction, RayHit2D& hit, float maxDistance) {
    Ray2D ray(origin, direction, maxDistance);
    return Raycast2D(ray, hit);
}

std::vector<RayHit2D> RayCaster::RaycastAll2D(const Ray2D& ray) {
    std::vector<RayHit2D> hits;
    hits.reserve(m_maxRaycastHits);
    
    RayHit2D hit;
    if (Raycast2D(ray, hit)) {
        hits.push_back(hit);
    }
    
    return hits;
}

std::vector<RayHit2D> RayCaster::RaycastAll2D(const Vector2& origin, const Vector2& direction, float maxDistance) {
    Ray2D ray(origin, direction, maxDistance);
    return RaycastAll2D(ray);
}

Ray3D RayCaster::ScreenPointToRay(const Vector2& screenPoint, const CameraComponent* camera, int screenWidth, int screenHeight) {
    if (!camera) {
        Logger::Error("RayCaster::ScreenPointToRay - Camera is null");
        return Ray3D(Vector3::Zero, Vector3::Forward);
    }
    
    float x = (2.0f * screenPoint.x) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * screenPoint.y) / screenHeight;
    
    Vector3 rayOrigin = Vector3(0.0f, 5.0f, 10.0f); // Camera position (simplified)
    Vector3 rayDirection = Vector3(x, y, -1.0f).Normalized(); // Direction towards screen point
    
    Logger::Debug("Screen point (" + std::to_string(screenPoint.x) + ", " + std::to_string(screenPoint.y) + 
                 ") converted to world ray: origin(" + std::to_string(rayOrigin.x) + ", " + 
                 std::to_string(rayOrigin.y) + ", " + std::to_string(rayOrigin.z) + 
                 ") direction(" + std::to_string(rayDirection.x) + ", " + 
                 std::to_string(rayDirection.y) + ", " + std::to_string(rayDirection.z) + ")");
    
    return Ray3D(rayOrigin, rayDirection);
}

bool RayCaster::ScreenPointRaycast(const Vector2& screenPoint, const CameraComponent* camera, int screenWidth, int screenHeight, RayHit3D& hit) {
    Ray3D ray = ScreenPointToRay(screenPoint, camera, screenWidth, screenHeight);
    return Raycast3D(ray, hit);
}

bool RayCaster::IsPointInSphere(const Vector3& point, const Vector3& sphereCenter, float sphereRadius) {
    float distanceSquared = (point - sphereCenter).LengthSquared();
    return distanceSquared <= (sphereRadius * sphereRadius);
}

bool RayCaster::IsPointInBox(const Vector3& point, const Vector3& boxCenter, const Vector3& boxSize) {
    Vector3 halfSize = boxSize * 0.5f;
    Vector3 localPoint = point - boxCenter;
    
    return (std::abs(localPoint.x) <= halfSize.x &&
            std::abs(localPoint.y) <= halfSize.y &&
            std::abs(localPoint.z) <= halfSize.z);
}

bool RayCaster::IsPointInCircle(const Vector2& point, const Vector2& circleCenter, float circleRadius) {
    float distanceSquared = (point - circleCenter).LengthSquared();
    return distanceSquared <= (circleRadius * circleRadius);
}

bool RayCaster::IsPointInRect(const Vector2& point, const Vector2& rectCenter, const Vector2& rectSize) {
    Vector2 halfSize = rectSize * 0.5f;
    Vector2 localPoint = point - rectCenter;
    
    return (std::abs(localPoint.x) <= halfSize.x &&
            std::abs(localPoint.y) <= halfSize.y);
}

bool RayCaster::RaycastAgainstRigidBody3D(const Ray3D& ray, RigidBody* body, RayHit3D& hit) {
    if (!body) return false;
    
    Vector3 rayEnd = ray.origin + ray.direction * ray.maxDistance;
    
    ContinuousCollisionInfo collisionInfo;
    if (ContinuousCollisionDetection::RaycastAgainstBody(ray.origin, rayEnd, body, collisionInfo)) {
        hit.hit = true;
        hit.point = collisionInfo.contactPoint;
        hit.normal = collisionInfo.normal;
        hit.distance = collisionInfo.timeOfImpact * ray.maxDistance;
        hit.rigidBody = body;
        return true;
    }
    
    return false;
}

bool RayCaster::RaycastAgainstRigidBody2D(const Ray2D& ray, RigidBody2D* body, RayHit2D& hit) {
    if (!body) return false;
    
    Vector2 bodyPos = body->GetPosition();
    float bodyRadius = body->GetColliderRadius();
    
    Vector2 toBody = bodyPos - ray.origin;
    float projLength = toBody.Dot(ray.direction);
    
    if (projLength < 0 || projLength > ray.maxDistance) return false;
    
    Vector2 closestPoint = ray.origin + ray.direction * projLength;
    float distanceToBody = (bodyPos - closestPoint).Length();
    
    if (distanceToBody <= bodyRadius) {
        float hitDistance = projLength - std::sqrt(bodyRadius * bodyRadius - distanceToBody * distanceToBody);
        if (hitDistance >= 0 && hitDistance <= ray.maxDistance) {
            hit.hit = true;
            hit.point = ray.origin + ray.direction * hitDistance;
            hit.normal = (hit.point - bodyPos).Normalized();
            hit.distance = hitDistance;
            hit.rigidBody2D = body;
            return true;
        }
    }
    
    return false;
}

Vector3 RayCaster::CalculateRayPoint3D(const Ray3D& ray, float distance) {
    return ray.origin + ray.direction * distance;
}

Vector2 RayCaster::CalculateRayPoint2D(const Ray2D& ray, float distance) {
    return ray.origin + ray.direction * distance;
}

bool RayCaster::SolveQuadratic(float a, float b, float c, float& t1, float& t2) {
    float discriminant = b * b - 4.0f * a * c;
    
    if (discriminant < 0.0f) {
        return false; // No real solutions
    }
    
    if (std::abs(a) < 1e-6f) {
        if (std::abs(b) < 1e-6f) {
            return false; // No solution
        }
        t1 = t2 = -c / b;
        return true;
    }
    
    float sqrtDiscriminant = std::sqrt(discriminant);
    t1 = (-b - sqrtDiscriminant) / (2.0f * a);
    t2 = (-b + sqrtDiscriminant) / (2.0f * a);
    
    return true;
}

}
