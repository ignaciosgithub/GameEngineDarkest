#include "MemoryManager.h"
#include "../Logging/Logger.h"
#include <iostream>
#include <iomanip>

namespace GameEngine {

MemoryManager& MemoryManager::Instance() {
    static MemoryManager instance;
    return instance;
}

void* MemoryManager::Allocate(size_t size, const std::string& tag) {
    void* ptr = std::malloc(size);
    if (!ptr) {
        Logger::Error("Failed to allocate " + std::to_string(size) + " bytes");
        return nullptr;
    }
    
    m_allocations[ptr] = {size, tag};
    m_totalAllocated += size;
    m_allocationCount++;
    
    Logger::Debug("Allocated " + std::to_string(size) + " bytes (" + tag + ")");
    return ptr;
}

void MemoryManager::Deallocate(void* ptr) {
    if (!ptr) return;
    
    auto it = m_allocations.find(ptr);
    if (it != m_allocations.end()) {
        m_totalAllocated -= it->second.size;
        m_allocationCount--;
        
        Logger::Debug("Deallocated " + std::to_string(it->second.size) + " bytes (" + it->second.tag + ")");
        m_allocations.erase(it);
    } else {
        Logger::Warning("Attempting to deallocate untracked memory");
    }
    
    std::free(ptr);
}

void MemoryManager::PrintMemoryReport() const {
    Logger::Info("=== Memory Report ===");
    Logger::Info("Total Allocated: " + std::to_string(m_totalAllocated) + " bytes");
    Logger::Info("Active Allocations: " + std::to_string(m_allocationCount));
    
    if (!m_allocations.empty()) {
        Logger::Info("Active allocations by tag:");
        std::unordered_map<std::string, size_t> tagTotals;
        
        for (const auto& allocation : m_allocations) {
            tagTotals[allocation.second.tag] += allocation.second.size;
        }
        
        for (const auto& tagTotal : tagTotals) {
            Logger::Info("  " + tagTotal.first + ": " + std::to_string(tagTotal.second) + " bytes");
        }
    }
    
    Logger::Info("=====================");
}

}

#ifdef TRACK_MEMORY
void* operator new(size_t size) {
    return GameEngine::MemoryManager::Instance().Allocate(size, "Global New");
}

void* operator new[](size_t size) {
    return GameEngine::MemoryManager::Instance().Allocate(size, "Global New[]");
}

void operator delete(void* ptr) noexcept {
    GameEngine::MemoryManager::Instance().Deallocate(ptr);
}

void operator delete[](void* ptr) noexcept {
    GameEngine::MemoryManager::Instance().Deallocate(ptr);
}
#endif
