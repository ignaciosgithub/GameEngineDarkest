#include "RenderManager.h"
#include "../Core/Logging/Logger.h"

namespace GameEngine {

RenderManager::RenderManager() = default;
RenderManager::~RenderManager() = default;

bool RenderManager::Initialize(int width, int height) {
    m_width = width;
    m_height = height;
    
    Logger::Info("Initializing Render Manager...");
    
    m_deferredPipeline = std::make_unique<DeferredRenderPipeline>();
    if (!m_deferredPipeline->Initialize(width, height)) {
        Logger::Error("Failed to initialize deferred rendering pipeline");
        return false;
    }
    
    SetPipeline(RenderPipelineType::Deferred);
    
    Logger::Info("Render Manager initialized successfully");
    return true;
}

void RenderManager::Shutdown() {
    Logger::Info("Shutting down Render Manager");
    
    if (m_deferredPipeline) {
        m_deferredPipeline->Shutdown();
        m_deferredPipeline.reset();
    }
    
    m_currentPipeline = nullptr;
}

void RenderManager::SetPipeline(RenderPipelineType type) {
    switch (type) {
        case RenderPipelineType::Deferred:
            m_currentPipeline = m_deferredPipeline.get();
            m_currentPipelineType = RenderPipelineType::Deferred;
            Logger::Info("Switched to Deferred Rendering Pipeline");
            break;
        case RenderPipelineType::Forward:
            Logger::Warning("Forward rendering pipeline not implemented yet");
            break;
    }
}

void RenderManager::BeginFrame(const RenderData& renderData) {
    if (m_currentPipeline) {
        m_currentPipeline->BeginFrame(renderData);
    }
}

void RenderManager::Render(World* world) {
    if (m_currentPipeline) {
        m_currentPipeline->Render(world);
    }
}

void RenderManager::EndFrame() {
    if (m_currentPipeline) {
        m_currentPipeline->EndFrame();
    }
}

void RenderManager::Resize(int width, int height) {
    m_width = width;
    m_height = height;
    
    if (m_deferredPipeline) {
        m_deferredPipeline->Resize(width, height);
    }
}

std::shared_ptr<Texture> RenderManager::GetFinalTexture() const {
    if (m_currentPipeline) {
        return m_currentPipeline->GetFinalTexture();
    }
    return nullptr;
}

}
