#pragma once

#include "../../Core/Math/Vector3.h"
#include <vector>
#include <memory>

namespace GameEngine {
    class RigidBody;
    
    struct AABB {
        Vector3 min;
        Vector3 max;
        
        AABB() = default;
        AABB(const Vector3& min, const Vector3& max) : min(min), max(max) {}
        
        bool Contains(const Vector3& point) const;
        bool Intersects(const AABB& other) const;
        Vector3 GetCenter() const { return (min + max) * 0.5f; }
        Vector3 GetSize() const { return max - min; }
    };
    
    class OctreeNode {
    public:
        OctreeNode(const AABB& bounds, int depth = 0, int maxDepth = 6);
        ~OctreeNode();
        
        void Insert(RigidBody* body);
        void Remove(RigidBody* body);
        void Clear();
        
        void Query(const AABB& bounds, std::vector<RigidBody*>& results) const;
        void QuerySphere(const Vector3& center, float radius, std::vector<RigidBody*>& results) const;
        
        bool IsLeaf() const { return m_children[0] == nullptr; }
        int GetDepth() const { return m_depth; }
        const AABB& GetBounds() const { return m_bounds; }
        
    private:
        void Subdivide();
        AABB GetChildBounds(int childIndex) const;
        int GetChildIndex(const Vector3& point) const;
        AABB GetBodyAABB(RigidBody* body) const;
        
        static const int MAX_OBJECTS_PER_NODE = 10;
        static const int MAX_DEPTH = 6;
        
        AABB m_bounds;
        int m_depth;
        int m_maxDepth;
        
        std::vector<RigidBody*> m_objects;
        std::unique_ptr<OctreeNode> m_children[8];
    };
    
    class Octree {
    public:
        Octree(const AABB& worldBounds);
        ~Octree();
        
        void Insert(RigidBody* body);
        void Remove(RigidBody* body);
        void Update(RigidBody* body);
        void Clear();
        
        void Query(const AABB& bounds, std::vector<RigidBody*>& results) const;
        void QuerySphere(const Vector3& center, float radius, std::vector<RigidBody*>& results) const;
        
        // Get all potential collision pairs
        void GetCollisionPairs(std::vector<std::pair<RigidBody*, RigidBody*>>& pairs) const;
        
        const AABB& GetWorldBounds() const { return m_worldBounds; }
        
    private:
        AABB m_worldBounds;
        std::unique_ptr<OctreeNode> m_root;
    };
}
