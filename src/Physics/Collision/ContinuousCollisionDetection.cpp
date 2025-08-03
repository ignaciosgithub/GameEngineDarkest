#include "ContinuousCollisionDetection.h"
#include "../RigidBody/RigidBody.h"
#include "../../Core/Components/ColliderComponent.h"
#include "../Colliders/ColliderShape.h"
#include "../../Core/Logging/Logger.h"
#include <cmath>
#include <algorithm>

namespace GameEngine {

bool ContinuousCollisionDetection::CheckContinuousCollision(RigidBody* bodyA, RigidBody* bodyB, 
                                                          float deltaTime, ContinuousCollisionInfo& info) {
    if (!bodyA || !bodyB) return false;
    
    if (bodyA->IsStatic() && bodyB->IsStatic()) return false;
    
    float speedThreshold = 10.0f; // Units per second
    float speedA = bodyA->GetVelocity().Length();
    float speedB = bodyB->GetVelocity().Length();
    
    if (speedA < speedThreshold && speedB < speedThreshold) {
        return false; // Use discrete collision detection for slow objects
    }
    
    ColliderComponent* colliderA = bodyA->GetColliderComponent();
    ColliderComponent* colliderB = bodyB->GetColliderComponent();
    
    if (!colliderA || !colliderB || !colliderA->HasCollider() || !colliderB->HasCollider()) {
        return false;
    }
    
    auto shapeA = colliderA->GetColliderShape();
    auto shapeB = colliderB->GetColliderShape();
    
    ColliderShapeType typeA = shapeA->GetType();
    ColliderShapeType typeB = shapeB->GetType();
    
    if (typeA == ColliderShapeType::Sphere && typeB == ColliderShapeType::Sphere) {
        return SphereSphereSwept(bodyA, bodyB, deltaTime, info);
    }
    else if (typeA == ColliderShapeType::Box && typeB == ColliderShapeType::Box) {
        return BoxBoxSwept(bodyA, bodyB, deltaTime, info);
    }
    
    Vector3 startA = bodyA->GetPosition();
    Vector3 endA = startA + bodyA->GetVelocity() * deltaTime;
    
    if (RaycastAgainstBody(startA, endA, bodyB, info)) {
        return true;
    }
    
    Vector3 startB = bodyB->GetPosition();
    Vector3 endB = startB + bodyB->GetVelocity() * deltaTime;
    
    return RaycastAgainstBody(startB, endB, bodyA, info);
}

bool ContinuousCollisionDetection::SphereSphereSwept(RigidBody* bodyA, RigidBody* bodyB, 
                                                   float deltaTime, ContinuousCollisionInfo& info) {
    Vector3 posA = bodyA->GetPosition();
    Vector3 posB = bodyB->GetPosition();
    Vector3 velA = bodyA->GetVelocity();
    Vector3 velB = bodyB->GetVelocity();
    
    auto sphereA = std::static_pointer_cast<SphereCollider>(bodyA->GetColliderComponent()->GetColliderShape());
    auto sphereB = std::static_pointer_cast<SphereCollider>(bodyB->GetColliderComponent()->GetColliderShape());
    
    float radiusA = sphereA->GetRadius();
    float radiusB = sphereB->GetRadius();
    float combinedRadius = radiusA + radiusB;
    
    Vector3 relativePos = posA - posB;
    Vector3 relativeVel = velA - velB;
    
    float a = relativeVel.Dot(relativeVel);
    float b = 2.0f * relativePos.Dot(relativeVel);
    float c = relativePos.Dot(relativePos) - combinedRadius * combinedRadius;
    
    float t1, t2;
    if (!SolveQuadratic(a, b, c, t1, t2)) {
        return false; // No collision
    }
    
    float collisionTime = -1.0f;
    if (t1 >= 0.0f && t1 <= deltaTime) {
        collisionTime = t1;
    } else if (t2 >= 0.0f && t2 <= deltaTime) {
        collisionTime = t2;
    }
    
    if (collisionTime < 0.0f) {
        return false; // No collision within time step
    }
    
    info.hasCollision = true;
    info.timeOfImpact = collisionTime / deltaTime;
    
    Vector3 posAAtCollision = posA + velA * collisionTime;
    Vector3 posBAtCollision = posB + velB * collisionTime;
    Vector3 direction = posAAtCollision - posBAtCollision;
    
    if (direction.Length() > 0.0f) {
        info.normal = direction.Normalized();
    } else {
        info.normal = Vector3::Up; // Default normal
    }
    
    info.contactPoint = posBAtCollision + info.normal * radiusB;
    info.penetration = 0.0f; // No penetration at time of impact
    
    return true;
}

bool ContinuousCollisionDetection::BoxBoxSwept(RigidBody* bodyA, RigidBody* bodyB, 
                                             float deltaTime, ContinuousCollisionInfo& info) {
    Vector3 posA = bodyA->GetPosition();
    Vector3 posB = bodyB->GetPosition();
    Vector3 velA = bodyA->GetVelocity();
    Vector3 velB = bodyB->GetVelocity();
    auto boxA = std::static_pointer_cast<BoxCollider>(bodyA->GetColliderComponent()->GetColliderShape());
    auto boxB = std::static_pointer_cast<BoxCollider>(bodyB->GetColliderComponent()->GetColliderShape());
    
    Vector3 sizeA = boxA->GetHalfExtents() * 2.0f;
    Vector3 sizeB = boxB->GetHalfExtents() * 2.0f;
    
    Vector3 expandedSize = sizeA + sizeB;
    Vector3 minB = posB - expandedSize * 0.5f;
    Vector3 maxB = posB + expandedSize * 0.5f;
    
    Vector3 relativeVel = velA - velB;
    
    float tMin = 0.0f;
    float tMax = deltaTime;
    
    for (int i = 0; i < 3; ++i) {
        float pos = (i == 0) ? posA.x : (i == 1) ? posA.y : posA.z;
        float vel = (i == 0) ? relativeVel.x : (i == 1) ? relativeVel.y : relativeVel.z;
        float minVal = (i == 0) ? minB.x : (i == 1) ? minB.y : minB.z;
        float maxVal = (i == 0) ? maxB.x : (i == 1) ? maxB.y : maxB.z;
        
        if (std::abs(vel) < 1e-6f) {
            if (pos < minVal || pos > maxVal) {
                return false; // No intersection
            }
        } else {
            float t1 = (minVal - pos) / vel;
            float t2 = (maxVal - pos) / vel;
            
            if (t1 > t2) std::swap(t1, t2);
            
            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);
            
            if (tMin > tMax) {
                return false; // No intersection
            }
        }
    }
    
    if (tMin >= 0.0f && tMin <= deltaTime) {
        info.hasCollision = true;
        info.timeOfImpact = tMin / deltaTime;
        
        Vector3 posAAtCollision = posA + velA * tMin;
        Vector3 posBAtCollision = posB + velB * tMin;
        
        Vector3 direction = posAAtCollision - posBAtCollision;
        if (direction.Length() > 0.0f) {
            info.normal = direction.Normalized();
        } else {
            info.normal = Vector3::Up;
        }
        
        info.contactPoint = posAAtCollision;
        info.penetration = 0.0f;
        
        return true;
    }
    
    return false;
}

bool ContinuousCollisionDetection::RaycastAgainstBody(const Vector3& rayStart, const Vector3& rayEnd, 
                                                    RigidBody* body, ContinuousCollisionInfo& info) {
    Vector3 rayDir = rayEnd - rayStart;
    float rayLength = rayDir.Length();
    
    if (rayLength < 1e-6f) return false;
    
    rayDir = rayDir / rayLength;
    
    Vector3 bodyPos = body->GetPosition();
    
    ColliderComponent* collider = body->GetColliderComponent();
    if (!collider || !collider->HasCollider()) {
        return false;
    }
    
    auto shape = collider->GetColliderShape();
    
    if (shape->GetType() == ColliderShapeType::Sphere) {
        auto sphereCollider = std::static_pointer_cast<SphereCollider>(shape);
        float radius = sphereCollider->GetRadius();
        Vector3 toSphere = rayStart - bodyPos;
        
        float a = rayDir.Dot(rayDir);
        float b = 2.0f * toSphere.Dot(rayDir);
        float c = toSphere.Dot(toSphere) - radius * radius;
        
        float t1, t2;
        if (SolveQuadratic(a, b, c, t1, t2)) {
            float t = (t1 >= 0.0f) ? t1 : t2;
            if (t >= 0.0f && t <= rayLength) {
                info.hasCollision = true;
                info.timeOfImpact = t / rayLength;
                info.contactPoint = rayStart + rayDir * t;
                info.normal = (info.contactPoint - bodyPos).Normalized();
                info.penetration = 0.0f;
                return true;
            }
        }
    }
    
    return false;
}

float ContinuousCollisionDetection::CalculateTimeOfImpact(RigidBody* bodyA, RigidBody* bodyB, float deltaTime) {
    ContinuousCollisionInfo info;
    if (CheckContinuousCollision(bodyA, bodyB, deltaTime, info)) {
        return info.timeOfImpact * deltaTime;
    }
    return deltaTime; // No collision, return full time step
}

bool ContinuousCollisionDetection::SolveQuadratic(float a, float b, float c, float& t1, float& t2) {
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

Vector3 ContinuousCollisionDetection::GetBodyVelocityAtTime(RigidBody* body, float /*time*/) {
    return body->GetVelocity();
}

Vector3 ContinuousCollisionDetection::GetBodyPositionAtTime(RigidBody* body, float time, float /*deltaTime*/) {
    return body->GetPosition() + body->GetVelocity() * time;
}

}
