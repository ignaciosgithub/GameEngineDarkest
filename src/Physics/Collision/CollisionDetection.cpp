#include "CollisionDetection.h"
#include "../RigidBody/RigidBody.h"
#include "../../Core/Components/ColliderComponent.h"
#include "../../Core/Components/TransformComponent.h"
#include "../Colliders/ColliderShape.h"
#include "../Spatial/Octree.h"
#include "../../Core/Math/Vector3.h"
#include "../../Core/Math/Matrix4.h"
#include "../../Core/Math/Quaternion.h"
#include "../../Core/Logging/Logger.h"
#include <cmath>
#include <memory>

namespace GameEngine {

Vector3 CollisionDetection::TransformPoint(const Vector3& localPoint, const Vector3& position, const Quaternion& rotation, const Vector3& scale) {
    Vector3 scaledPoint = Vector3(localPoint.x * scale.x, localPoint.y * scale.y, localPoint.z * scale.z);
    Vector3 rotatedPoint = rotation.RotateVector(scaledPoint);
    return position + rotatedPoint;
}

float CollisionDetection::TransformRadius(float localRadius, const Vector3& scale) {
    return localRadius * std::max({scale.x, scale.y, scale.z});
}

Vector3 CollisionDetection::TransformHalfExtents(const Vector3& localHalfExtents, const Vector3& scale) {
    return Vector3(localHalfExtents.x * scale.x, localHalfExtents.y * scale.y, localHalfExtents.z * scale.z);
}

Matrix4 CollisionDetection::GetOrientationMatrix(const Quaternion& rotation) {
    (void)rotation; // Suppress unused parameter warning
    return Matrix4::Identity();
}

bool CollisionDetection::CheckCollision(RigidBody* bodyA, RigidBody* bodyB) {
    return CheckCollision(bodyA, bodyB, nullptr);
}

bool CollisionDetection::CheckCollision(RigidBody* bodyA, RigidBody* bodyB, Octree* octree) {
    if (!bodyA || !bodyB) return false;
    
    ColliderComponent* colliderA = bodyA->GetColliderComponent();
    ColliderComponent* colliderB = bodyB->GetColliderComponent();
    
    if (!colliderA || !colliderB || !colliderA->HasCollider() || !colliderB->HasCollider()) {
        return false;
    }
    
    if (octree) {
        std::vector<RigidBody*> potentialCollisions;
        
        Vector3 posA = bodyA->GetPosition();
        auto transformA = bodyA->GetTransformComponent();
        Vector3 scaleA = transformA ? transformA->transform.GetWorldScale() : Vector3::One;
        
        Vector3 minA, maxA;
        auto shapeA = colliderA->GetColliderShape();
        if (shapeA->GetType() == ColliderShapeType::Sphere) {
            auto sphereA = std::static_pointer_cast<SphereCollider>(shapeA);
            float radius = TransformRadius(sphereA->GetRadius(), scaleA);
            minA = posA - Vector3(radius, radius, radius);
            maxA = posA + Vector3(radius, radius, radius);
        } else if (shapeA->GetType() == ColliderShapeType::Box) {
            auto boxA = std::static_pointer_cast<BoxCollider>(shapeA);
            Vector3 halfExtents = TransformHalfExtents(boxA->GetHalfExtents(), scaleA);
            minA = posA - halfExtents;
            maxA = posA + halfExtents;
        } else {
            Vector3 conservativeSize = Vector3(2.0f, 2.0f, 2.0f) * scaleA;
            minA = posA - conservativeSize;
            maxA = posA + conservativeSize;
        }
        
        AABB queryAABB(minA, maxA);
        octree->Query(queryAABB, potentialCollisions);
        
        bool foundInOctree = false;
        for (RigidBody* body : potentialCollisions) {
            if (body == bodyB) {
                foundInOctree = true;
                break;
            }
        }
        
        if (!foundInOctree) {
            return false;
        }
        
        Logger::Debug("Octree optimization: Found potential collision between bodies, proceeding with detailed check");
    }
    
    CollisionInfo info;
    bool hasCollision = false;
    
    auto shapeA = colliderA->GetColliderShape();
    auto shapeB = colliderB->GetColliderShape();
    
    ColliderShapeType typeA = shapeA->GetType();
    ColliderShapeType typeB = shapeB->GetType();
    
    if (typeA == ColliderShapeType::Sphere && typeB == ColliderShapeType::Sphere) {
        hasCollision = SphereVsSphere(bodyA, bodyB, info);
    }
    else if (typeA == ColliderShapeType::Box && typeB == ColliderShapeType::Box) {
        hasCollision = BoxVsBox(bodyA, bodyB, info);
    }
    else if ((typeA == ColliderShapeType::Sphere && typeB == ColliderShapeType::Box) ||
             (typeA == ColliderShapeType::Box && typeB == ColliderShapeType::Sphere)) {
        hasCollision = SphereVsBox(bodyA, bodyB, info);
    }
    else if (typeA == ColliderShapeType::ConvexHull && typeB == ColliderShapeType::ConvexHull) {
        hasCollision = ConvexHullVsConvexHull(bodyA, bodyB, info);
    }
    else if (typeA == ColliderShapeType::TriangleMesh && typeB == ColliderShapeType::TriangleMesh) {
        hasCollision = TriangleMeshVsTriangleMesh(bodyA, bodyB, info);
    }
    else if ((typeA == ColliderShapeType::ConvexHull && typeB == ColliderShapeType::TriangleMesh) ||
             (typeA == ColliderShapeType::TriangleMesh && typeB == ColliderShapeType::ConvexHull)) {
        hasCollision = ConvexHullVsTriangleMesh(bodyA, bodyB, info);
    }
    else if ((typeA == ColliderShapeType::Sphere && typeB == ColliderShapeType::ConvexHull) ||
             (typeA == ColliderShapeType::ConvexHull && typeB == ColliderShapeType::Sphere)) {
        hasCollision = SphereVsConvexHull(bodyA, bodyB, info);
    }
    else if ((typeA == ColliderShapeType::Box && typeB == ColliderShapeType::ConvexHull) ||
             (typeA == ColliderShapeType::ConvexHull && typeB == ColliderShapeType::Box)) {
        hasCollision = BoxVsConvexHull(bodyA, bodyB, info);
    }
    
    if (hasCollision) {
        ResolveCollision(bodyA, bodyB, info);
    }
    
    return hasCollision;
}

bool CollisionDetection::SphereVsSphere(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info) {
    Vector3 posA = bodyA->GetPosition();
    Vector3 posB = bodyB->GetPosition();
    
    auto sphereA = std::static_pointer_cast<SphereCollider>(bodyA->GetColliderComponent()->GetColliderShape());
    auto sphereB = std::static_pointer_cast<SphereCollider>(bodyB->GetColliderComponent()->GetColliderShape());
    
    Vector3 scaleA = bodyA->GetTransformComponent() ? bodyA->GetTransformComponent()->transform.GetWorldScale() : Vector3::One;
    Vector3 scaleB = bodyB->GetTransformComponent() ? bodyB->GetTransformComponent()->transform.GetWorldScale() : Vector3::One;
    
    float radiusA = TransformRadius(sphereA->GetRadius(), scaleA);
    float radiusB = TransformRadius(sphereB->GetRadius(), scaleB);
    
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
    
    auto boxA = std::static_pointer_cast<BoxCollider>(bodyA->GetColliderComponent()->GetColliderShape());
    auto boxB = std::static_pointer_cast<BoxCollider>(bodyB->GetColliderComponent()->GetColliderShape());
    
    Vector3 scaleA = bodyA->GetTransformComponent() ? bodyA->GetTransformComponent()->transform.GetWorldScale() : Vector3::One;
    Vector3 scaleB = bodyB->GetTransformComponent() ? bodyB->GetTransformComponent()->transform.GetWorldScale() : Vector3::One;
    
    Vector3 sizeA = TransformHalfExtents(boxA->GetHalfExtents(), scaleA) * 2.0f;
    Vector3 sizeB = TransformHalfExtents(boxB->GetHalfExtents(), scaleB) * 2.0f;
    
    Quaternion rotA = bodyA->GetTransformComponent() ? bodyA->GetTransformComponent()->transform.GetWorldRotation() : Quaternion::Identity();
    Quaternion rotB = bodyB->GetTransformComponent() ? bodyB->GetTransformComponent()->transform.GetWorldRotation() : Quaternion::Identity();
    
    (void)rotA; // Suppress unused variable warning
    (void)rotB; // Suppress unused variable warning
    
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
    auto shapeA = bodyA->GetColliderComponent()->GetColliderShape();
    auto shapeB = bodyB->GetColliderComponent()->GetColliderShape();
    
    RigidBody* sphere = (shapeA->GetType() == ColliderShapeType::Sphere) ? bodyA : bodyB;
    RigidBody* box = (shapeA->GetType() == ColliderShapeType::Box) ? bodyA : bodyB;
    
    auto sphereCollider = std::static_pointer_cast<SphereCollider>(sphere->GetColliderComponent()->GetColliderShape());
    auto boxCollider = std::static_pointer_cast<BoxCollider>(box->GetColliderComponent()->GetColliderShape());
    
    Vector3 spherePos = sphere->GetPosition();
    Vector3 boxPos = box->GetPosition();
    
    Vector3 sphereScale = sphere->GetTransformComponent() ? sphere->GetTransformComponent()->transform.GetWorldScale() : Vector3::One;
    Vector3 boxScale = box->GetTransformComponent() ? box->GetTransformComponent()->transform.GetWorldScale() : Vector3::One;
    
    Vector3 boxSize = TransformHalfExtents(boxCollider->GetHalfExtents(), boxScale) * 2.0f;
    float sphereRadius = TransformRadius(sphereCollider->GetRadius(), sphereScale);
    
    Quaternion boxRotation = box->GetTransformComponent() ? box->GetTransformComponent()->transform.GetWorldRotation() : Quaternion::Identity();
    
    (void)boxRotation; // Suppress unused variable warning
    
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

bool CollisionDetection::ConvexHullVsConvexHull(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info) {
    auto colliderA = bodyA->GetColliderComponent();
    auto colliderB = bodyB->GetColliderComponent();
    
    if (!colliderA || !colliderB || !colliderA->HasCollider() || !colliderB->HasCollider()) {
        return false;
    }
    
    auto convexHullA = std::static_pointer_cast<ConvexHullCollider>(colliderA->GetColliderShape());
    auto convexHullB = std::static_pointer_cast<ConvexHullCollider>(colliderB->GetColliderShape());
    
    auto transformA = bodyA->GetTransformComponent();
    auto transformB = bodyB->GetTransformComponent();
    
    if (!transformA || !transformB) {
        return false;
    }
    
    const auto& localVerticesA = convexHullA->GetVertices();
    const auto& localVerticesB = convexHullB->GetVertices();
    
    if (localVerticesA.empty() || localVerticesB.empty()) {
        return false;
    }
    
    Matrix4 worldMatrixA = transformA->transform.GetLocalToWorldMatrix();
    Matrix4 worldMatrixB = transformB->transform.GetLocalToWorldMatrix();
    
    std::vector<Vector3> worldVerticesA;
    std::vector<Vector3> worldVerticesB;
    
    for (const auto& vertex : localVerticesA) {
        worldVerticesA.push_back(worldMatrixA * vertex);
    }
    
    for (const auto& vertex : localVerticesB) {
        worldVerticesB.push_back(worldMatrixB * vertex);
    }
    
    Vector3 minA = worldVerticesA[0], maxA = worldVerticesA[0];
    Vector3 minB = worldVerticesB[0], maxB = worldVerticesB[0];
    
    for (const auto& vertex : worldVerticesA) {
        minA = Vector3(std::min(minA.x, vertex.x), std::min(minA.y, vertex.y), std::min(minA.z, vertex.z));
        maxA = Vector3(std::max(maxA.x, vertex.x), std::max(maxA.y, vertex.y), std::max(maxA.z, vertex.z));
    }
    
    for (const auto& vertex : worldVerticesB) {
        minB = Vector3(std::min(minB.x, vertex.x), std::min(minB.y, vertex.y), std::min(minB.z, vertex.z));
        maxB = Vector3(std::max(maxB.x, vertex.x), std::max(maxB.y, vertex.y), std::max(maxB.z, vertex.z));
    }
    
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
            info.normal = (bodyA->GetPosition().x < bodyB->GetPosition().x) ? Vector3(-1, 0, 0) : Vector3(1, 0, 0);
        } else if (overlap.y <= overlap.z) {
            info.penetration = overlap.y;
            info.normal = (bodyA->GetPosition().y < bodyB->GetPosition().y) ? Vector3(0, -1, 0) : Vector3(0, 1, 0);
        } else {
            info.penetration = overlap.z;
            info.normal = (bodyA->GetPosition().z < bodyB->GetPosition().z) ? Vector3(0, 0, -1) : Vector3(0, 0, 1);
        }
        
        info.contactPoint = bodyA->GetPosition() + (bodyB->GetPosition() - bodyA->GetPosition()) * 0.5f;
        return true;
    }
    
    return false;
}

bool CollisionDetection::TriangleMeshVsTriangleMesh(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info) {
    auto colliderA = bodyA->GetColliderComponent();
    auto colliderB = bodyB->GetColliderComponent();
    
    if (!colliderA || !colliderB || !colliderA->HasCollider() || !colliderB->HasCollider()) {
        return false;
    }
    
    auto triangleMeshA = std::static_pointer_cast<TriangleMeshCollider>(colliderA->GetColliderShape());
    auto triangleMeshB = std::static_pointer_cast<TriangleMeshCollider>(colliderB->GetColliderShape());
    
    auto transformA = bodyA->GetTransformComponent();
    auto transformB = bodyB->GetTransformComponent();
    
    if (!transformA || !transformB) {
        return false;
    }
    
    const auto& localVerticesA = triangleMeshA->GetVertices();
    const auto& localVerticesB = triangleMeshB->GetVertices();
    
    if (localVerticesA.empty() || localVerticesB.empty()) {
        return false;
    }
    
    Matrix4 worldMatrixA = transformA->transform.GetLocalToWorldMatrix();
    Matrix4 worldMatrixB = transformB->transform.GetLocalToWorldMatrix();
    
    std::vector<Vector3> worldVerticesA;
    std::vector<Vector3> worldVerticesB;
    
    for (const auto& vertex : localVerticesA) {
        worldVerticesA.push_back(worldMatrixA * vertex);
    }
    
    for (const auto& vertex : localVerticesB) {
        worldVerticesB.push_back(worldMatrixB * vertex);
    }
    
    Vector3 minA = worldVerticesA[0], maxA = worldVerticesA[0];
    Vector3 minB = worldVerticesB[0], maxB = worldVerticesB[0];
    
    for (const auto& vertex : worldVerticesA) {
        minA = Vector3(std::min(minA.x, vertex.x), std::min(minA.y, vertex.y), std::min(minA.z, vertex.z));
        maxA = Vector3(std::max(maxA.x, vertex.x), std::max(maxA.y, vertex.y), std::max(maxA.z, vertex.z));
    }
    
    for (const auto& vertex : worldVerticesB) {
        minB = Vector3(std::min(minB.x, vertex.x), std::min(minB.y, vertex.y), std::min(minB.z, vertex.z));
        maxB = Vector3(std::max(maxB.x, vertex.x), std::max(maxB.y, vertex.y), std::max(maxB.z, vertex.z));
    }
    
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
            info.normal = (bodyA->GetPosition().x < bodyB->GetPosition().x) ? Vector3(-1, 0, 0) : Vector3(1, 0, 0);
        } else if (overlap.y <= overlap.z) {
            info.penetration = overlap.y;
            info.normal = (bodyA->GetPosition().y < bodyB->GetPosition().y) ? Vector3(0, -1, 0) : Vector3(0, 1, 0);
        } else {
            info.penetration = overlap.z;
            info.normal = (bodyA->GetPosition().z < bodyB->GetPosition().z) ? Vector3(0, 0, -1) : Vector3(0, 0, 1);
        }
        
        info.contactPoint = bodyA->GetPosition() + (bodyB->GetPosition() - bodyA->GetPosition()) * 0.5f;
        return true;
    }
    
    return false;
}

bool CollisionDetection::ConvexHullVsTriangleMesh(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info) {
    auto shapeA = bodyA->GetColliderComponent()->GetColliderShape();
    auto shapeB = bodyB->GetColliderComponent()->GetColliderShape();
    
    RigidBody* convexHull = (shapeA->GetType() == ColliderShapeType::ConvexHull) ? bodyA : bodyB;
    RigidBody* triangleMesh = (shapeA->GetType() == ColliderShapeType::TriangleMesh) ? bodyA : bodyB;
    
    auto convexHullCollider = std::static_pointer_cast<ConvexHullCollider>(convexHull->GetColliderComponent()->GetColliderShape());
    auto triangleMeshCollider = std::static_pointer_cast<TriangleMeshCollider>(triangleMesh->GetColliderComponent()->GetColliderShape());
    
    auto transformConvex = convexHull->GetTransformComponent();
    auto transformMesh = triangleMesh->GetTransformComponent();
    
    if (!transformConvex || !transformMesh) {
        return false;
    }
    
    const auto& localVerticesConvex = convexHullCollider->GetVertices();
    const auto& localVerticesMesh = triangleMeshCollider->GetVertices();
    
    if (localVerticesConvex.empty() || localVerticesMesh.empty()) {
        return false;
    }
    
    Matrix4 worldMatrixConvex = transformConvex->transform.GetLocalToWorldMatrix();
    Matrix4 worldMatrixMesh = transformMesh->transform.GetLocalToWorldMatrix();
    
    std::vector<Vector3> worldVerticesConvex;
    std::vector<Vector3> worldVerticesMesh;
    
    for (const auto& vertex : localVerticesConvex) {
        worldVerticesConvex.push_back(worldMatrixConvex * vertex);
    }
    
    for (const auto& vertex : localVerticesMesh) {
        worldVerticesMesh.push_back(worldMatrixMesh * vertex);
    }
    
    Vector3 minConvex = worldVerticesConvex[0], maxConvex = worldVerticesConvex[0];
    Vector3 minMesh = worldVerticesMesh[0], maxMesh = worldVerticesMesh[0];
    
    for (const auto& vertex : worldVerticesConvex) {
        minConvex = Vector3(std::min(minConvex.x, vertex.x), std::min(minConvex.y, vertex.y), std::min(minConvex.z, vertex.z));
        maxConvex = Vector3(std::max(maxConvex.x, vertex.x), std::max(maxConvex.y, vertex.y), std::max(maxConvex.z, vertex.z));
    }
    
    for (const auto& vertex : worldVerticesMesh) {
        minMesh = Vector3(std::min(minMesh.x, vertex.x), std::min(minMesh.y, vertex.y), std::min(minMesh.z, vertex.z));
        maxMesh = Vector3(std::max(maxMesh.x, vertex.x), std::max(maxMesh.y, vertex.y), std::max(maxMesh.z, vertex.z));
    }
    
    bool overlapX = (minConvex.x <= maxMesh.x) && (maxConvex.x >= minMesh.x);
    bool overlapY = (minConvex.y <= maxMesh.y) && (maxConvex.y >= minMesh.y);
    bool overlapZ = (minConvex.z <= maxMesh.z) && (maxConvex.z >= minMesh.z);
    
    if (overlapX && overlapY && overlapZ) {
        info.hasCollision = true;
        
        Vector3 overlap;
        overlap.x = std::min(maxConvex.x - minMesh.x, maxMesh.x - minConvex.x);
        overlap.y = std::min(maxConvex.y - minMesh.y, maxMesh.y - minConvex.y);
        overlap.z = std::min(maxConvex.z - minMesh.z, maxMesh.z - minConvex.z);
        
        if (overlap.x <= overlap.y && overlap.x <= overlap.z) {
            info.penetration = overlap.x;
            info.normal = (convexHull->GetPosition().x < triangleMesh->GetPosition().x) ? Vector3(-1, 0, 0) : Vector3(1, 0, 0);
        } else if (overlap.y <= overlap.z) {
            info.penetration = overlap.y;
            info.normal = (convexHull->GetPosition().y < triangleMesh->GetPosition().y) ? Vector3(0, -1, 0) : Vector3(0, 1, 0);
        } else {
            info.penetration = overlap.z;
            info.normal = (convexHull->GetPosition().z < triangleMesh->GetPosition().z) ? Vector3(0, 0, -1) : Vector3(0, 0, 1);
        }
        
        if (convexHull == bodyB) {
            info.normal = -info.normal;
        }
        
        info.contactPoint = convexHull->GetPosition() + (triangleMesh->GetPosition() - convexHull->GetPosition()) * 0.5f;
        return true;
    }
    
    return false;
}

bool CollisionDetection::SphereVsConvexHull(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info) {
    auto shapeA = bodyA->GetColliderComponent()->GetColliderShape();
    auto shapeB = bodyB->GetColliderComponent()->GetColliderShape();
    
    RigidBody* sphere = (shapeA->GetType() == ColliderShapeType::Sphere) ? bodyA : bodyB;
    RigidBody* convexHull = (shapeA->GetType() == ColliderShapeType::ConvexHull) ? bodyA : bodyB;
    
    auto sphereCollider = std::static_pointer_cast<SphereCollider>(sphere->GetColliderComponent()->GetColliderShape());
    auto convexHullCollider = std::static_pointer_cast<ConvexHullCollider>(convexHull->GetColliderComponent()->GetColliderShape());
    
    Vector3 spherePos = sphere->GetPosition();
    
    auto transformSphere = sphere->GetTransformComponent();
    auto transformConvex = convexHull->GetTransformComponent();
    
    Vector3 sphereScale = transformSphere ? transformSphere->transform.GetWorldScale() : Vector3::One;
    float sphereRadius = TransformRadius(sphereCollider->GetRadius(), sphereScale);
    
    if (!transformConvex) {
        return false;
    }
    
    const auto& localVertices = convexHullCollider->GetVertices();
    
    if (localVertices.empty()) {
        return false;
    }
    
    Matrix4 worldMatrix = transformConvex->transform.GetLocalToWorldMatrix();
    std::vector<Vector3> worldVertices;
    
    for (const auto& vertex : localVertices) {
        worldVertices.push_back(worldMatrix * vertex);
    }
    
    Vector3 closestPoint = worldVertices[0];
    float minDistanceSquared = (spherePos - closestPoint).LengthSquared();
    
    for (const auto& vertex : worldVertices) {
        float distanceSquared = (spherePos - vertex).LengthSquared();
        if (distanceSquared < minDistanceSquared) {
            minDistanceSquared = distanceSquared;
            closestPoint = vertex;
        }
    }
    
    float distance = std::sqrt(minDistanceSquared);
    
    if (distance < sphereRadius) {
        info.hasCollision = true;
        info.penetration = sphereRadius - distance;
        
        if (distance > 0.0f) {
            info.normal = (spherePos - closestPoint) / distance;
        } else {
            info.normal = Vector3::Up; // Default normal when sphere center is at vertex
        }
        
        if (sphere == bodyB) {
            info.normal = -info.normal;
        }
        
        info.contactPoint = closestPoint;
        return true;
    }
    
    return false;
}

bool CollisionDetection::BoxVsConvexHull(RigidBody* bodyA, RigidBody* bodyB, CollisionInfo& info) {
    auto shapeA = bodyA->GetColliderComponent()->GetColliderShape();
    auto shapeB = bodyB->GetColliderComponent()->GetColliderShape();
    
    RigidBody* box = (shapeA->GetType() == ColliderShapeType::Box) ? bodyA : bodyB;
    RigidBody* convexHull = (shapeA->GetType() == ColliderShapeType::ConvexHull) ? bodyA : bodyB;
    
    auto boxCollider = std::static_pointer_cast<BoxCollider>(box->GetColliderComponent()->GetColliderShape());
    auto convexHullCollider = std::static_pointer_cast<ConvexHullCollider>(convexHull->GetColliderComponent()->GetColliderShape());
    
    Vector3 boxPos = box->GetPosition();
    
    auto transformBox = box->GetTransformComponent();
    auto transformConvex = convexHull->GetTransformComponent();
    
    Vector3 boxScale = transformBox ? transformBox->transform.GetWorldScale() : Vector3::One;
    Vector3 boxSize = TransformHalfExtents(boxCollider->GetHalfExtents(), boxScale) * 2.0f;
    
    if (!transformConvex) {
        return false;
    }
    
    const auto& localVertices = convexHullCollider->GetVertices();
    
    if (localVertices.empty()) {
        return false;
    }
    
    Matrix4 worldMatrix = transformConvex->transform.GetLocalToWorldMatrix();
    std::vector<Vector3> worldVertices;
    
    for (const auto& vertex : localVertices) {
        worldVertices.push_back(worldMatrix * vertex);
    }
    
    Vector3 boxMin = boxPos - boxSize * 0.5f;
    Vector3 boxMax = boxPos + boxSize * 0.5f;
    
    Vector3 convexMin = worldVertices[0], convexMax = worldVertices[0];
    
    for (const auto& vertex : worldVertices) {
        convexMin = Vector3(std::min(convexMin.x, vertex.x), std::min(convexMin.y, vertex.y), std::min(convexMin.z, vertex.z));
        convexMax = Vector3(std::max(convexMax.x, vertex.x), std::max(convexMax.y, vertex.y), std::max(convexMax.z, vertex.z));
    }
    
    bool overlapX = (boxMin.x <= convexMax.x) && (boxMax.x >= convexMin.x);
    bool overlapY = (boxMin.y <= convexMax.y) && (boxMax.y >= convexMin.y);
    bool overlapZ = (boxMin.z <= convexMax.z) && (boxMax.z >= convexMin.z);
    
    if (overlapX && overlapY && overlapZ) {
        info.hasCollision = true;
        
        Vector3 overlap;
        overlap.x = std::min(boxMax.x - convexMin.x, convexMax.x - boxMin.x);
        overlap.y = std::min(boxMax.y - convexMin.y, convexMax.y - boxMin.y);
        overlap.z = std::min(boxMax.z - convexMin.z, convexMax.z - boxMin.z);
        
        if (overlap.x <= overlap.y && overlap.x <= overlap.z) {
            info.penetration = overlap.x;
            info.normal = (boxPos.x < convexHull->GetPosition().x) ? Vector3(-1, 0, 0) : Vector3(1, 0, 0);
        } else if (overlap.y <= overlap.z) {
            info.penetration = overlap.y;
            info.normal = (boxPos.y < convexHull->GetPosition().y) ? Vector3(0, -1, 0) : Vector3(0, 1, 0);
        } else {
            info.penetration = overlap.z;
            info.normal = (boxPos.z < convexHull->GetPosition().z) ? Vector3(0, 0, -1) : Vector3(0, 0, 1);
        }
        
        if (box == bodyB) {
            info.normal = -info.normal;
        }
        
        info.contactPoint = boxPos + (convexHull->GetPosition() - boxPos) * 0.5f;
        return true;
    }
    
    return false;
}

}
