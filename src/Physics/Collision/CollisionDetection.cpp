#include "CollisionDetection.h"
#include "../RigidBody/RigidBody.h"
#include "../../Core/Math/Vector3.h"
#include "../../Core/Logging/Logger.h"
#include <cmath>

namespace GameEngine {

bool CollisionDetection::CheckCollision(RigidBody* bodyA, RigidBody* bodyB) {
    if (!bodyA || !bodyB) return false;
    
    CollisionInfo info;
    bool hasCollision = false;
    
    if (bodyA->GetColliderType() == ColliderType::Sphere && bodyB->GetColliderType() == ColliderType::Sphere) {
        hasCollision = SphereVsSphere(bodyA, bodyB, info);
    }
    else if (bodyA->GetColliderType() == ColliderType::Box && bodyB->GetColliderType() == ColliderType::Box) {
        hasCollision = BoxVsBox(bodyA, bodyB, info);
    }
    else if ((bodyA->GetColliderType() == ColliderType::Sphere && bodyB->GetColliderType() == ColliderType::Box) ||
             (bodyA->GetColliderType() == ColliderType::Box && bodyB->GetColliderType() == ColliderType::Sphere)) {
        hasCollision = SphereVsBox(bodyA, bodyB, info);
    }
    
    if (hasCollision) {
        ResolveCollision(bodyA, bodyB, info);
    }
    
    return hasCollision;
}

bool CollisionDetection::SphereVsSphere(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info) {
    Vector3 posA = bodyA->GetPosition();
    Vector3 posB = bodyB->GetPosition();
    
    float radiusA = bodyA->GetColliderSize().x; // Assuming x component is radius
    float radiusB = bodyB->GetColliderSize().x;
    
    Vector3 direction = posB - posA;
    float distance = direction.Length();
    float combinedRadius = radiusA + radiusB;
    
    if (distance < combinedRadius) {
        info.hasCollision = true;
        info.penetration = combinedRadius - distance;
        
        if (distance > 0.0f) {
            info.normal = direction / distance;
        } else {
            info.normal = Vector3::Up; // Default normal when spheres are at same position
        }
        
        info.contactPoint = posA + info.normal * radiusA;
        return true;
    }
    
    return false;
}

bool CollisionDetection::BoxVsBox(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info) {
    Vector3 posA = bodyA->GetPosition();
    Vector3 posB = bodyB->GetPosition();
    Vector3 sizeA = bodyA->GetColliderSize();
    Vector3 sizeB = bodyB->GetColliderSize();
    
    Vector3 minA = posA - sizeA * 0.5f;
    Vector3 maxA = posA + sizeA * 0.5f;
    Vector3 minB = posB - sizeB * 0.5f;
    Vector3 maxB = posB + sizeB * 0.5f;
    
    bool overlapX = (minA.x <= maxB.x) && (maxA.x >= minB.x);
    bool overlapY = (minA.y <= maxB.y) && (maxA.y >= minB.y);
    bool overlapZ = (minA.z <= maxB.z) && (maxA.z >= minB.z);
    
    if (overlapX && overlapY && overlapZ) {
        info.hasCollision = true;
        
        Vector3 overlap;
        overlap.x = std::min(maxA.x - minB.x, maxB.x - minA.x);
        overlap.y = std::min(maxA.y - minB.y, maxB.y - minA.y);
        overlap.z = std::min(maxA.z - minB.z, maxB.z - minA.z);
        
        if (overlap.x <= overlap.y && overlap.x <= overlap.z) {
            info.penetration = overlap.x;
            info.normal = (posA.x < posB.x) ? Vector3(-1, 0, 0) : Vector3(1, 0, 0);
        } else if (overlap.y <= overlap.z) {
            info.penetration = overlap.y;
            info.normal = (posA.y < posB.y) ? Vector3(0, -1, 0) : Vector3(0, 1, 0);
        } else {
            info.penetration = overlap.z;
            info.normal = (posA.z < posB.z) ? Vector3(0, 0, -1) : Vector3(0, 0, 1);
        }
        
        info.contactPoint = posA + (posB - posA) * 0.5f;
        return true;
    }
    
    return false;
}

bool CollisionDetection::SphereVsBox(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info) {
    RigidBody* sphere = (bodyA->GetColliderType() == ColliderType::Sphere) ? bodyA : bodyB;
    RigidBody* box = (bodyA->GetColliderType() == ColliderType::Box) ? bodyA : bodyB;
    
    Vector3 spherePos = sphere->GetPosition();
    Vector3 boxPos = box->GetPosition();
    Vector3 boxSize = box->GetColliderSize();
    float sphereRadius = sphere->GetColliderSize().x;
    
    Vector3 boxMin = boxPos - boxSize * 0.5f;
    Vector3 boxMax = boxPos + boxSize * 0.5f;
    
    Vector3 closestPoint;
    closestPoint.x = std::max(boxMin.x, std::min(spherePos.x, boxMax.x));
    closestPoint.y = std::max(boxMin.y, std::min(spherePos.y, boxMax.y));
    closestPoint.z = std::max(boxMin.z, std::min(spherePos.z, boxMax.z));
    
    Vector3 direction = spherePos - closestPoint;
    float distance = direction.Length();
    
    if (distance < sphereRadius) {
        info.hasCollision = true;
        info.penetration = sphereRadius - distance;
        
        if (distance > 0.0f) {
            info.normal = direction / distance;
        } else {
            info.normal = Vector3::Up; // Default normal when sphere center is inside box
        }
        
        if (sphere == bodyB) {
            info.normal = -info.normal;
        }
        
        info.contactPoint = closestPoint;
        return true;
    }
    
    return false;
}

void CollisionDetection::ResolveCollision(RigidBody* bodyA, RigidBody* bodyB, const CollisionInfo& info) {
    if (!info.hasCollision) return;
    
    float totalInvMass = bodyA->GetInverseMass() + bodyB->GetInverseMass();
    if (totalInvMass <= 0.0f) return; // Both objects are static
    
    Vector3 separation = info.normal * info.penetration;
    float separationA = bodyA->GetInverseMass() / totalInvMass;
    float separationB = bodyB->GetInverseMass() / totalInvMass;
    
    if (!bodyA->IsStatic()) {
        bodyA->SetPosition(bodyA->GetPosition() - separation * separationA);
    }
    if (!bodyB->IsStatic()) {
        bodyB->SetPosition(bodyB->GetPosition() + separation * separationB);
    }
    
    Vector3 relativeVelocity = bodyB->GetVelocity() - bodyA->GetVelocity();
    float velocityAlongNormal = relativeVelocity.Dot(info.normal);
    
    if (velocityAlongNormal > 0) return;
    
    float restitution = std::min(bodyA->GetRestitution(), bodyB->GetRestitution());
    
    float impulseScalar = -(1 + restitution) * velocityAlongNormal / totalInvMass;
    
    Vector3 impulse = info.normal * impulseScalar;
    
    if (!bodyA->IsStatic()) {
        bodyA->SetVelocity(bodyA->GetVelocity() - impulse * bodyA->GetInverseMass());
    }
    if (!bodyB->IsStatic()) {
        bodyB->SetVelocity(bodyB->GetVelocity() + impulse * bodyB->GetInverseMass());
    }
}

}
