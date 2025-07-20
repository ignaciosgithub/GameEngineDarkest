#pragma once

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <string>

namespace GameEngine {
    class MemoryManager {
    public:
        static MemoryManager& Instance();
        
        void* Allocate(size_t size, const std::string& tag = "Unknown");
        void Deallocate(void* ptr);
        
        template<typename T, typename... Args>
        T* New(const std::string& tag = "Unknown", Args&&... args);
        
        template<typename T>
        void Delete(T* ptr);
        
        // Memory tracking
        size_t GetTotalAllocated() const { return m_totalAllocated; }
        size_t GetAllocationCount() const { return m_allocationCount; }
        
        void PrintMemoryReport() const;
        
    private:
        MemoryManager() = default;
        ~MemoryManager() = default;
        
        struct AllocationInfo {
            size_t size;
            std::string tag;
        };
        
        std::unordered_map<void*, AllocationInfo> m_allocations;
        size_t m_totalAllocated = 0;
        size_t m_allocationCount = 0;
    };
    
    // Template implementations
    template<typename T, typename... Args>
    T* MemoryManager::New(const std::string& tag, Args&&... args) {
        void* memory = Allocate(sizeof(T), tag);
        return new(memory) T(std::forward<Args>(args)...);
    }
    
    template<typename T>
    void MemoryManager::Delete(T* ptr) {
        if (ptr) {
            ptr->~T();
            Deallocate(ptr);
        }
    }
}

// Global new/delete overrides for tracking (optional)
#ifdef TRACK_MEMORY
void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
#endif
