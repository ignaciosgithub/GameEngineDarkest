#pragma once

#include <cstdint>

namespace GameEngine {
    using EntityID = uint32_t;
    
    constexpr EntityID INVALID_ENTITY = 0;
    
    class Entity {
    public:
        Entity() : m_id(INVALID_ENTITY) {}
        explicit Entity(EntityID id) : m_id(id) {}
        
        EntityID GetID() const { return m_id; }
        bool IsValid() const { return m_id != INVALID_ENTITY; }
        
        bool operator==(const Entity& other) const { return m_id == other.m_id; }
        bool operator!=(const Entity& other) const { return m_id != other.m_id; }
        bool operator<(const Entity& other) const { return m_id < other.m_id; }
        
    private:
        EntityID m_id;
    };
}
