#pragma once

#include "../../../Core/Math/Vector2.h"
#include <vector>
#include <memory>

namespace GameEngine {
    class RigidBody2D;
    
    struct QuadTreeBounds {
        Vector2 center;
        Vector2 halfSize;
        
        QuadTreeBounds() = default;
        QuadTreeBounds(const Vector2& center, const Vector2& halfSize) 
            : center(center), halfSize(halfSize) {}
        
        bool Contains(const Vector2& point) const;
        bool Intersects(const QuadTreeBounds& other) const;
        QuadTreeBounds GetQuadrant(int quadrant) const; // 0=NE, 1=NW, 2=SW, 3=SE
    };
    
    class QuadTree {
    public:
        static const int MAX_OBJECTS = 10;
        static const int MAX_LEVELS = 5;
        
        QuadTree(int level = 0, const QuadTreeBounds& bounds = QuadTreeBounds());
        ~QuadTree();
        
        void Clear();
        void Insert(RigidBody2D* body);
        void Retrieve(std::vector<RigidBody2D*>& returnObjects, RigidBody2D* body);
        void Retrieve(std::vector<RigidBody2D*>& returnObjects, const QuadTreeBounds& bounds);
        
        // Debug information
        int GetObjectCount() const;
        int GetNodeCount() const;
        void GetAllBounds(std::vector<QuadTreeBounds>& bounds) const;
        
    private:
        int m_level;
        QuadTreeBounds m_bounds;
        std::vector<RigidBody2D*> m_objects;
        std::unique_ptr<QuadTree> m_nodes[4]; // NE, NW, SW, SE
        
        void Split();
        int GetIndex(RigidBody2D* body);
        QuadTreeBounds GetBodyBounds(RigidBody2D* body);
    };
}
