#include "CollisionDetection2D.h"
#include "../RigidBody2D.h"
#include "../Colliders/Collider2D.h"
#include "../Colliders/CircleCollider2D.h"
#include "../Colliders/BoxCollider2D.h"
#include "../../../Core/Logging/Logger.h"
#include <cmath>
#include <algorithm>

namespace GameEngine {

bool CollisionDetection2D::CheckCollision(RigidBody2D* bodyA, RigidBody2D* bodyB, CollisionInfo2D& info) {
    if (!bodyA || !bodyB) return false;
    
    info.bodyA = bodyA;
    info.bodyB = bodyB;
    info.hasCollision = false;
    
    Collider2DType typeA = bodyA->GetColliderType();
    Collider2DType typeB = bodyB->GetColliderType();
    
    if (typeA == Collider2DType::Circle && typeB == Collider2DType::Circle) {
        CircleCollider2D circleA(bodyA->GetColliderRadius());
        CircleCollider2D circleB(bodyB->GetColliderRadius());
        return CircleVsCircle(&circleA, bodyA, &circleB, bodyB, info);
    }
    else if (typeA == Collider2DType::Box && typeB == Collider2DType::Box) {
        BoxCollider2D boxA(bodyA->GetColliderSize());
        BoxCollider2D boxB(bodyB->GetColliderSize());
        return BoxVsBox(&boxA, bodyA, &boxB, bodyB, info);
    }
    else if (typeA == Collider2DType::Circle && typeB == Collider2DType::Box) {
        CircleCollider2D circle(bodyA->GetColliderRadius());
        BoxCollider2D box(bodyB->GetColliderSize());
        return CircleVsBox(&circle, bodyA, &box, bodyB, info);
    }
    else if (typeA == Collider2DType::Box && typeB == Collider2DType::Circle) {
        CircleCollider2D circle(bodyB->GetColliderRadius());
        BoxCollider2D box(bodyA->GetColliderSize());
        bool result = CircleVsBox(&circle, bodyB, &box, bodyA, info);
        if (result) {
            std::swap(info.bodyA, info.bodyB);
            info.normal = -info.normal;
        }
        return result;
    }
    
    return false;
}

bool CollisionDetection2D::CircleVsCircle(const CircleCollider2D* circleA, const RigidBody2D* bodyA,
                                         const CircleCollider2D* circleB, const RigidBody2D* bodyB,
                                         CollisionInfo2D& info) {
    Vector2 centerA = bodyA->GetPosition() + circleA->GetOffset();
    Vector2 centerB = bodyB->GetPosition() + circleB->GetOffset();
    
    Vector2 distance = centerB - centerA;
    float distanceLength = distance.Length();
    float radiusSum = circleA->GetRadius() + circleB->GetRadius();
    
    if (distanceLength < radiusSum) {
        info.hasCollision = true;
        info.penetration = radiusSum - distanceLength;
        
        if (distanceLength > 0.0f) {
            info.normal = distance / distanceLength;
        } else {
            info.normal = Vector2(1.0f, 0.0f); // Default normal
        }
        
        info.contactPoint = centerA + info.normal * circleA->GetRadius();
        return true;
    }
    
    return false;
}

bool CollisionDetection2D::BoxVsBox(const BoxCollider2D* boxA, const RigidBody2D* bodyA,
                                   const BoxCollider2D* boxB, const RigidBody2D* bodyB,
                                   CollisionInfo2D& info) {
    
    Vector2 centerA = bodyA->GetPosition() + boxA->GetOffset();
    Vector2 centerB = bodyB->GetPosition() + boxB->GetOffset();
    Vector2 halfSizeA = boxA->GetHalfSize();
    Vector2 halfSizeB = boxB->GetHalfSize();
    
    Vector2 minA = centerA - halfSizeA;
    Vector2 maxA = centerA + halfSizeA;
    Vector2 minB = centerB - halfSizeB;
    Vector2 maxB = centerB + halfSizeB;
    
    if (AABBVsAABB(minA, maxA, minB, maxB)) {
        info.hasCollision = true;
        
        Vector2 distance = centerB - centerA;
        Vector2 overlap;
        overlap.x = (halfSizeA.x + halfSizeB.x) - std::abs(distance.x);
        overlap.y = (halfSizeA.y + halfSizeB.y) - std::abs(distance.y);
        
        if (overlap.x < overlap.y) {
            info.penetration = overlap.x;
            info.normal = Vector2(distance.x > 0 ? 1.0f : -1.0f, 0.0f);
        } else {
            info.penetration = overlap.y;
            info.normal = Vector2(0.0f, distance.y > 0 ? 1.0f : -1.0f);
        }
        
        info.contactPoint = centerA + info.normal * (halfSizeA.x * std::abs(info.normal.x) + halfSizeA.y * std::abs(info.normal.y));
        return true;
    }
    
    return false;
}

bool CollisionDetection2D::CircleVsBox(const CircleCollider2D* circle, const RigidBody2D* circleBody,
                                      const BoxCollider2D* box, const RigidBody2D* boxBody,
                                      CollisionInfo2D& info) {
    Vector2 circleCenter = circleBody->GetPosition() + circle->GetOffset();
    Vector2 boxCenter = boxBody->GetPosition() + box->GetOffset();
    Vector2 halfSize = box->GetHalfSize();
    
    Vector2 closest;
    closest.x = std::max(boxCenter.x - halfSize.x, std::min(circleCenter.x, boxCenter.x + halfSize.x));
    closest.y = std::max(boxCenter.y - halfSize.y, std::min(circleCenter.y, boxCenter.y + halfSize.y));
    
    Vector2 distance = circleCenter - closest;
    float distanceLength = distance.Length();
    
    if (distanceLength < circle->GetRadius()) {
        info.hasCollision = true;
        info.penetration = circle->GetRadius() - distanceLength;
        
        if (distanceLength > 0.0f) {
            info.normal = distance / distanceLength;
        } else {
            Vector2 toCenter = circleCenter - boxCenter;
            Vector2 absToCenter(std::abs(toCenter.x), std::abs(toCenter.y));
            Vector2 overlap = halfSize - absToCenter;
            
            if (overlap.x < overlap.y) {
                info.normal = Vector2(toCenter.x > 0 ? 1.0f : -1.0f, 0.0f);
                info.penetration = circle->GetRadius() + overlap.x;
            } else {
                info.normal = Vector2(0.0f, toCenter.y > 0 ? 1.0f : -1.0f);
                info.penetration = circle->GetRadius() + overlap.y;
            }
        }
        
        info.contactPoint = closest;
        return true;
    }
    
    return false;
}

bool CollisionDetection2D::AABBVsAABB(const Vector2& minA, const Vector2& maxA,
                                     const Vector2& minB, const Vector2& maxB) {
    return (minA.x <= maxB.x && maxA.x >= minB.x &&
            minA.y <= maxB.y && maxA.y >= minB.y);
}

bool CollisionDetection2D::PointInCircle(const Vector2& point, const Vector2& center, float radius) {
    Vector2 distance = point - center;
    return distance.LengthSquared() <= radius * radius;
}

bool CollisionDetection2D::PointInBox(const Vector2& point, const BoxCollider2D* box, const RigidBody2D* body) {
    Vector2 center = body->GetPosition() + box->GetOffset();
    Vector2 halfSize = box->GetHalfSize();
    Vector2 min = center - halfSize;
    Vector2 max = center + halfSize;
    
    return (point.x >= min.x && point.x <= max.x &&
            point.y >= min.y && point.y <= max.y);
}

float CollisionDetection2D::DistancePointToLine(const Vector2& point, const Vector2& lineStart, const Vector2& lineEnd) {
    Vector2 line = lineEnd - lineStart;
    Vector2 pointToStart = point - lineStart;
    
    float lineLength = line.LengthSquared();
    if (lineLength == 0.0f) {
        return pointToStart.Length();
    }
    
    float t = std::max(0.0f, std::min(1.0f, pointToStart.Dot(line) / lineLength));
    Vector2 projection = lineStart + line * t;
    return (point - projection).Length();
}

Vector2 CollisionDetection2D::ClosestPointOnLine(const Vector2& point, const Vector2& lineStart, const Vector2& lineEnd) {
    Vector2 line = lineEnd - lineStart;
    Vector2 pointToStart = point - lineStart;
    
    float lineLength = line.LengthSquared();
    if (lineLength == 0.0f) {
        return lineStart;
    }
    
    float t = std::max(0.0f, std::min(1.0f, pointToStart.Dot(line) / lineLength));
    return lineStart + line * t;
}

void CollisionDetection2D::ResolveCollision(RigidBody2D* bodyA, RigidBody2D* bodyB, const CollisionInfo2D& info) {
    if (!info.hasCollision || !bodyA || !bodyB) return;
    
    Vector2 separation = info.normal * info.penetration;
    
    if (bodyA->IsDynamic() && bodyB->IsDynamic()) {
        float totalMass = bodyA->GetMass() + bodyB->GetMass();
        float ratioA = bodyB->GetMass() / totalMass;
        float ratioB = bodyA->GetMass() / totalMass;
        
        bodyA->SetPosition(bodyA->GetPosition() - separation * ratioA);
        bodyB->SetPosition(bodyB->GetPosition() + separation * ratioB);
    } else if (bodyA->IsDynamic()) {
        bodyA->SetPosition(bodyA->GetPosition() - separation);
    } else if (bodyB->IsDynamic()) {
        bodyB->SetPosition(bodyB->GetPosition() + separation);
    }
    
    Vector2 relativeVelocity = bodyB->GetVelocity() - bodyA->GetVelocity();
    float velocityAlongNormal = relativeVelocity.Dot(info.normal);
    
    if (velocityAlongNormal > 0) return;
    
    float restitution = std::min(bodyA->GetRestitution(), bodyB->GetRestitution());
    
    float impulseScalar = -(1 + restitution) * velocityAlongNormal;
    impulseScalar /= bodyA->GetInverseMass() + bodyB->GetInverseMass();
    
    Vector2 impulse = info.normal * impulseScalar;
    
    if (bodyA->IsDynamic()) {
        bodyA->SetVelocity(bodyA->GetVelocity() - impulse * bodyA->GetInverseMass());
    }
    if (bodyB->IsDynamic()) {
        bodyB->SetVelocity(bodyB->GetVelocity() + impulse * bodyB->GetInverseMass());
    }
    
    Vector2 tangent = relativeVelocity - info.normal * velocityAlongNormal;
    if (tangent.LengthSquared() > 0.0f) {
        tangent = tangent.Normalized();
        
        float frictionImpulse = -relativeVelocity.Dot(tangent);
        frictionImpulse /= bodyA->GetInverseMass() + bodyB->GetInverseMass();
        
        float friction = std::sqrt(bodyA->GetFriction() * bodyB->GetFriction());
        
        Vector2 frictionVector;
        if (std::abs(frictionImpulse) < impulseScalar * friction) {
            frictionVector = tangent * frictionImpulse;
        } else {
            frictionVector = tangent * -impulseScalar * friction;
        }
        
        if (bodyA->IsDynamic()) {
            bodyA->SetVelocity(bodyA->GetVelocity() - frictionVector * bodyA->GetInverseMass());
        }
        if (bodyB->IsDynamic()) {
            bodyB->SetVelocity(bodyB->GetVelocity() + frictionVector * bodyB->GetInverseMass());
        }
    }
}

}
