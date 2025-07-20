#pragma once

#include <typeindex>
#include <cstdint>

namespace GameEngine {
    using ComponentTypeID = std::type_index;
    
    template<typename T>
    ComponentTypeID GetComponentTypeID() {
        return std::type_index(typeid(T));
    }
    
    class IComponent {
    public:
        virtual ~IComponent() = default;
        virtual ComponentTypeID GetTypeID() const = 0;
    };
    
    template<typename T>
    class Component : public IComponent {
    public:
        ComponentTypeID GetTypeID() const override {
            return GetComponentTypeID<T>();
        }
    };
}
