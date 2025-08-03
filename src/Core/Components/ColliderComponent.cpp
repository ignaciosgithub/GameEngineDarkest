#include "ColliderComponent.h"
#include "MeshComponent.h"
#include "../../Physics/Colliders/ColliderShape.h"
#include "../../Rendering/Meshes/Mesh.h"
#include "../Logging/Logger.h"

namespace GameEngine {

ColliderComponent::ColliderComponent() {
    Logger::Debug("ColliderComponent created");
}

ColliderComponent::ColliderComponent(std::shared_ptr<ColliderShape> shape) 
    : m_colliderShape(shape) {
    Logger::Debug("ColliderComponent created with shape");
}

void ColliderComponent::SetColliderShape(std::shared_ptr<ColliderShape> shape) {
    m_colliderShape = shape;
}

void ColliderComponent::SetSphereCollider(float radius) {
    m_colliderShape = std::make_shared<SphereCollider>(radius);
}

void ColliderComponent::SetBoxCollider(const Vector3& halfExtents) {
    m_colliderShape = std::make_shared<BoxCollider>(halfExtents);
}

void ColliderComponent::SetCapsuleCollider(float radius, float height) {
    m_colliderShape = std::make_shared<CapsuleCollider>(radius, height);
}

void ColliderComponent::SetPlaneCollider(const Vector3& normal, float distance) {
    m_colliderShape = std::make_shared<PlaneCollider>(normal, distance);
}

void ColliderComponent::SetConvexHullCollider(const std::vector<Vector3>& vertices) {
    m_colliderShape = std::make_shared<ConvexHullCollider>(vertices);
}

void ColliderComponent::SetTriangleMeshCollider(const std::vector<Vector3>& vertices, const std::vector<unsigned int>& indices) {
    m_colliderShape = std::make_shared<TriangleMeshCollider>(vertices, indices);
}

void ColliderComponent::ClearCollider() {
    m_colliderShape.reset();
}

void ColliderComponent::GenerateFromMesh(MeshComponent* meshComponent, ColliderShapeType type) {
    if (!meshComponent || !meshComponent->HasMesh()) {
        Logger::Warning("Cannot generate collider from null or empty mesh");
        return;
    }

    auto mesh = meshComponent->GetMesh();
    const auto& vertices = mesh->GetVertices();
    const auto& indices = mesh->GetIndices();
    
    if (vertices.empty()) {
        Logger::Warning("Cannot generate collider from mesh with no vertices");
        return;
    }

    switch (type) {
        case ColliderShapeType::Box: {
            Vector3 min = vertices[0].position;
            Vector3 max = vertices[0].position;
            
            for (const auto& vertex : vertices) {
                min.x = std::min(min.x, vertex.position.x);
                min.y = std::min(min.y, vertex.position.y);
                min.z = std::min(min.z, vertex.position.z);
                
                max.x = std::max(max.x, vertex.position.x);
                max.y = std::max(max.y, vertex.position.y);
                max.z = std::max(max.z, vertex.position.z);
            }
            
            Vector3 halfExtents = (max - min) * 0.5f;
            SetBoxCollider(halfExtents);
            Logger::Info("Generated box collider from " + std::to_string(vertices.size()) + 
                        " vertices with half-extents: " + 
                        std::to_string(halfExtents.x) + ", " + 
                        std::to_string(halfExtents.y) + ", " + 
                        std::to_string(halfExtents.z));
            break;
        }
        case ColliderShapeType::Sphere: {
            Vector3 center = Vector3::Zero;
            for (const auto& vertex : vertices) {
                center = center + vertex.position;
            }
            center = center / static_cast<float>(vertices.size());
            
            float maxDistanceSquared = 0.0f;
            for (const auto& vertex : vertices) {
                Vector3 diff = vertex.position - center;
                float distanceSquared = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
                maxDistanceSquared = std::max(maxDistanceSquared, distanceSquared);
            }
            
            float radius = std::sqrt(maxDistanceSquared);
            SetSphereCollider(radius);
            Logger::Info("Generated sphere collider from " + std::to_string(vertices.size()) + 
                        " vertices with radius: " + std::to_string(radius));
            break;
        }
        case ColliderShapeType::ConvexHull: {
            std::vector<Vector3> positions;
            positions.reserve(vertices.size());
            
            for (const auto& vertex : vertices) {
                positions.push_back(vertex.position);
            }
            
            SetConvexHullCollider(positions);
            Logger::Info("Generated convex hull collider from " + std::to_string(vertices.size()) + 
                        " vertices using actual OBJ vertex data");
            break;
        }
        case ColliderShapeType::TriangleMesh: {
            std::vector<Vector3> positions;
            positions.reserve(vertices.size());
            
            for (const auto& vertex : vertices) {
                positions.push_back(vertex.position);
            }
            
            SetTriangleMeshCollider(positions, indices);
            Logger::Info("Generated triangle mesh collider from " + std::to_string(vertices.size()) + 
                        " vertices and " + std::to_string(indices.size()) + " indices using actual OBJ vertex data");
            break;
        }
        default:
            Logger::Warning("Unsupported collider type for mesh generation");
            break;
    }
}

}
