#include "Octree.h"
#include "../RigidBody/RigidBody.h"
#include "../../Core/Logging/Logger.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {

bool AABB::Contains(const Vector3& point) const {
    return point.x >= min.x && point.x <= max.x &&
           point.y >= min.y && point.y <= max.y &&
           point.z >= min.z && point.z <= max.z;
}

bool AABB::Intersects(const AABB& other) const {
    return min.x <= other.max.x && max.x >= other.min.x &&
           min.y <= other.max.y && max.y >= other.min.y &&
           min.z <= other.max.z && max.z >= other.min.z;
}

OctreeNode::OctreeNode(const AABB& bounds, int depth, int maxDepth)
    : m_bounds(bounds), m_depth(depth), m_maxDepth(maxDepth) {
    for (int i = 0; i < 8; ++i) {
        m_children[i] = nullptr;
    }
}

OctreeNode::~OctreeNode() {
    Clear();
}

void OctreeNode::Insert(RigidBody* body) {
    if (!body) return;
    
    AABB bodyAABB = GetBodyAABB(body);
    if (!m_bounds.Intersects(bodyAABB)) {
        return; // Body doesn't fit in this node
    }
    
    if (IsLeaf()) {
        m_objects.push_back(body);
        
        if (m_objects.size() > MAX_OBJECTS_PER_NODE && m_depth < m_maxDepth) {
            Subdivide();
            
            auto it = m_objects.begin();
            while (it != m_objects.end()) {
                bool inserted = false;
                AABB objAABB = GetBodyAABB(*it);
                
                for (int i = 0; i < 8; ++i) {
                    if (m_children[i] && m_children[i]->GetBounds().Intersects(objAABB)) {
                        m_children[i]->Insert(*it);
                        inserted = true;
                        break;
                    }
                }
                
                if (inserted) {
                    it = m_objects.erase(it);
                } else {
                    ++it; // Keep object in this node if it doesn't fit in any child
                }
            }
        }
    } else {
        bool inserted = false;
        for (int i = 0; i < 8; ++i) {
            if (m_children[i] && m_children[i]->GetBounds().Intersects(bodyAABB)) {
                m_children[i]->Insert(body);
                inserted = true;
                break;
            }
        }
        
        if (!inserted) {
            m_objects.push_back(body); // Keep in this node if doesn't fit in children
        }
    }
}

void OctreeNode::Remove(RigidBody* body) {
    if (!body) return;
    
    auto it = std::find(m_objects.begin(), m_objects.end(), body);
    if (it != m_objects.end()) {
        m_objects.erase(it);
        return;
    }
    
    if (!IsLeaf()) {
        for (int i = 0; i < 8; ++i) {
            if (m_children[i]) {
                m_children[i]->Remove(body);
            }
        }
    }
}

void OctreeNode::Clear() {
    m_objects.clear();
    
    for (int i = 0; i < 8; ++i) {
        m_children[i].reset();
    }
}

void OctreeNode::Query(const AABB& bounds, std::vector<RigidBody*>& results) const {
    if (!m_bounds.Intersects(bounds)) {
        return;
    }
    
    for (RigidBody* body : m_objects) {
        AABB bodyAABB = GetBodyAABB(body);
        if (bodyAABB.Intersects(bounds)) {
            results.push_back(body);
        }
    }
    
    if (!IsLeaf()) {
        for (int i = 0; i < 8; ++i) {
            if (m_children[i]) {
                m_children[i]->Query(bounds, results);
            }
        }
    }
}

void OctreeNode::QuerySphere(const Vector3& center, float radius, std::vector<RigidBody*>& results) const {
    Vector3 radiusVec(radius, radius, radius);
    AABB sphereAABB(center - radiusVec, center + radiusVec);
    
    if (!m_bounds.Intersects(sphereAABB)) {
        return;
    }
    
    for (RigidBody* body : m_objects) {
        Vector3 bodyPos = body->GetPosition();
        float distance = (bodyPos - center).Length();
        
        if (distance <= radius + 1.0f) { // Add some tolerance
            results.push_back(body);
        }
    }
    
    if (!IsLeaf()) {
        for (int i = 0; i < 8; ++i) {
            if (m_children[i]) {
                m_children[i]->QuerySphere(center, radius, results);
            }
        }
    }
}

void OctreeNode::Subdivide() {
    if (!IsLeaf()) return;
    
    for (int i = 0; i < 8; ++i) {
        AABB childBounds = GetChildBounds(i);
        m_children[i] = std::make_unique<OctreeNode>(childBounds, m_depth + 1, m_maxDepth);
    }
}

AABB OctreeNode::GetChildBounds(int childIndex) const {
    Vector3 center = m_bounds.GetCenter();
    Vector3 size = m_bounds.GetSize() * 0.5f;
    
    Vector3 offset;
    offset.x = (childIndex & 1) ? size.x * 0.5f : -size.x * 0.5f;
    offset.y = (childIndex & 2) ? size.y * 0.5f : -size.y * 0.5f;
    offset.z = (childIndex & 4) ? size.z * 0.5f : -size.z * 0.5f;
    
    Vector3 childCenter = center + offset;
    Vector3 halfSize = size * 0.5f;
    
    return AABB(childCenter - halfSize, childCenter + halfSize);
}

int OctreeNode::GetChildIndex(const Vector3& point) const {
    Vector3 center = m_bounds.GetCenter();
    int index = 0;
    
    if (point.x > center.x) index |= 1;
    if (point.y > center.y) index |= 2;
    if (point.z > center.z) index |= 4;
    
    return index;
}

AABB OctreeNode::GetBodyAABB(RigidBody* body) const {
    Vector3 pos = body->GetPosition();
    Vector3 size = body->GetColliderSize();
    
    Vector3 halfSize = size * 0.5f;
    return AABB(pos - halfSize, pos + halfSize);
}

Octree::Octree(const AABB& worldBounds) : m_worldBounds(worldBounds) {
    m_root = std::make_unique<OctreeNode>(worldBounds);
    Logger::Info("Octree initialized with bounds: min(" + 
                std::to_string(worldBounds.min.x) + ", " + 
                std::to_string(worldBounds.min.y) + ", " + 
                std::to_string(worldBounds.min.z) + ") max(" +
                std::to_string(worldBounds.max.x) + ", " + 
                std::to_string(worldBounds.max.y) + ", " + 
                std::to_string(worldBounds.max.z) + ")");
}

Octree::~Octree() = default;

void Octree::Insert(RigidBody* body) {
    if (m_root) {
        m_root->Insert(body);
    }
}

void Octree::Remove(RigidBody* body) {
    if (m_root) {
        m_root->Remove(body);
    }
}

void Octree::Update(RigidBody* body) {
    Remove(body);
    Insert(body);
}

void Octree::Clear() {
    if (m_root) {
        m_root->Clear();
    }
}

void Octree::Query(const AABB& bounds, std::vector<RigidBody*>& results) const {
    if (m_root) {
        m_root->Query(bounds, results);
    }
}

void Octree::QuerySphere(const Vector3& center, float radius, std::vector<RigidBody*>& results) const {
    if (m_root) {
        m_root->QuerySphere(center, radius, results);
    }
}

void Octree::GetCollisionPairs(std::vector<std::pair<RigidBody*, RigidBody*>>& pairs) const {
    std::vector<RigidBody*> allBodies;
    Query(m_worldBounds, allBodies);
    
    for (size_t i = 0; i < allBodies.size(); ++i) {
        for (size_t j = i + 1; j < allBodies.size(); ++j) {
            pairs.emplace_back(allBodies[i], allBodies[j]);
        }
    }
}

}
