#include "QuadTree.h"
#include "../RigidBody2D.h"
#include <algorithm>

namespace GameEngine {

bool QuadTreeBounds::Contains(const Vector2& point) const {
    return (point.x >= center.x - halfSize.x && point.x <= center.x + halfSize.x &&
            point.y >= center.y - halfSize.y && point.y <= center.y + halfSize.y);
}

bool QuadTreeBounds::Intersects(const QuadTreeBounds& other) const {
    return !(center.x - halfSize.x > other.center.x + other.halfSize.x ||
             center.x + halfSize.x < other.center.x - other.halfSize.x ||
             center.y - halfSize.y > other.center.y + other.halfSize.y ||
             center.y + halfSize.y < other.center.y - other.halfSize.y);
}

QuadTreeBounds QuadTreeBounds::GetQuadrant(int quadrant) const {
    Vector2 newHalfSize = halfSize * 0.5f;
    Vector2 newCenter = center;
    
    switch (quadrant) {
        case 0: // NE
            newCenter.x += newHalfSize.x;
            newCenter.y += newHalfSize.y;
            break;
        case 1: // NW
            newCenter.x -= newHalfSize.x;
            newCenter.y += newHalfSize.y;
            break;
        case 2: // SW
            newCenter.x -= newHalfSize.x;
            newCenter.y -= newHalfSize.y;
            break;
        case 3: // SE
            newCenter.x += newHalfSize.x;
            newCenter.y -= newHalfSize.y;
            break;
    }
    
    return QuadTreeBounds(newCenter, newHalfSize);
}

QuadTree::QuadTree(int level, const QuadTreeBounds& bounds) 
    : m_level(level), m_bounds(bounds) {
    for (int i = 0; i < 4; ++i) {
        m_nodes[i] = nullptr;
    }
}

QuadTree::~QuadTree() {
    Clear();
}

void QuadTree::Clear() {
    m_objects.clear();
    
    for (int i = 0; i < 4; ++i) {
        if (m_nodes[i]) {
            m_nodes[i]->Clear();
            m_nodes[i].reset();
        }
    }
}

void QuadTree::Insert(RigidBody2D* body) {
    if (!body) return;
    
    if (m_nodes[0]) {
        int index = GetIndex(body);
        if (index != -1) {
            m_nodes[index]->Insert(body);
            return;
        }
    }
    
    m_objects.push_back(body);
    
    if (m_objects.size() > MAX_OBJECTS && m_level < MAX_LEVELS) {
        if (!m_nodes[0]) {
            Split();
        }
        
        auto it = m_objects.begin();
        while (it != m_objects.end()) {
            int index = GetIndex(*it);
            if (index != -1) {
                m_nodes[index]->Insert(*it);
                it = m_objects.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void QuadTree::Retrieve(std::vector<RigidBody2D*>& returnObjects, RigidBody2D* body) {
    if (!body) return;
    
    QuadTreeBounds bodyBounds = GetBodyBounds(body);
    Retrieve(returnObjects, bodyBounds);
}

void QuadTree::Retrieve(std::vector<RigidBody2D*>& returnObjects, const QuadTreeBounds& bounds) {
    if (!m_bounds.Intersects(bounds)) return;
    
    for (RigidBody2D* obj : m_objects) {
        if (std::find(returnObjects.begin(), returnObjects.end(), obj) == returnObjects.end()) {
            returnObjects.push_back(obj);
        }
    }
    
    if (m_nodes[0]) {
        for (int i = 0; i < 4; ++i) {
            m_nodes[i]->Retrieve(returnObjects, bounds);
        }
    }
}

int QuadTree::GetObjectCount() const {
    int count = m_objects.size();
    
    if (m_nodes[0]) {
        for (int i = 0; i < 4; ++i) {
            count += m_nodes[i]->GetObjectCount();
        }
    }
    
    return count;
}

int QuadTree::GetNodeCount() const {
    int count = 1;
    
    if (m_nodes[0]) {
        for (int i = 0; i < 4; ++i) {
            count += m_nodes[i]->GetNodeCount();
        }
    }
    
    return count;
}

void QuadTree::GetAllBounds(std::vector<QuadTreeBounds>& bounds) const {
    bounds.push_back(m_bounds);
    
    if (m_nodes[0]) {
        for (int i = 0; i < 4; ++i) {
            m_nodes[i]->GetAllBounds(bounds);
        }
    }
}

void QuadTree::Split() {
    for (int i = 0; i < 4; ++i) {
        QuadTreeBounds quadrantBounds = m_bounds.GetQuadrant(i);
        m_nodes[i] = std::make_unique<QuadTree>(m_level + 1, quadrantBounds);
    }
}

int QuadTree::GetIndex(RigidBody2D* body) {
    if (!body) return -1;
    
    QuadTreeBounds bodyBounds = GetBodyBounds(body);
    
    for (int i = 0; i < 4; ++i) {
        QuadTreeBounds quadrantBounds = m_bounds.GetQuadrant(i);
        
        if (bodyBounds.center.x - bodyBounds.halfSize.x >= quadrantBounds.center.x - quadrantBounds.halfSize.x &&
            bodyBounds.center.x + bodyBounds.halfSize.x <= quadrantBounds.center.x + quadrantBounds.halfSize.x &&
            bodyBounds.center.y - bodyBounds.halfSize.y >= quadrantBounds.center.y - quadrantBounds.halfSize.y &&
            bodyBounds.center.y + bodyBounds.halfSize.y <= quadrantBounds.center.y + quadrantBounds.halfSize.y) {
            return i;
        }
    }
    
    return -1; // Doesn't fit completely in any quadrant
}

QuadTreeBounds QuadTree::GetBodyBounds(RigidBody2D* body) {
    Vector2 center = body->GetPosition();
    Vector2 halfSize;
    
    if (body->GetColliderType() == Collider2DType::Circle) {
        float radius = body->GetColliderRadius();
        halfSize = Vector2(radius, radius);
    } else if (body->GetColliderType() == Collider2DType::Box) {
        halfSize = body->GetColliderSize() * 0.5f;
    } else {
        halfSize = Vector2(0.5f, 0.5f);
    }
    
    return QuadTreeBounds(center, halfSize);
}

}
