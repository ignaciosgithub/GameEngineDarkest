#include "RenderManager.h"
#include "../Core/Logging/Logger.h"
#include "../Core/Profiling/Profiler.h"
#include "Core/GLDebug.h"

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
    
    m_forwardPipeline = std::make_unique<ForwardRenderPipeline>();
    m_forwardPipeline->Initialize(width, height);
    
    m_raytracingPipeline = std::make_unique<RaytracingPipeline>();
    m_raytracingPipeline->Initialize(width, height);

    EnableGLDebug();
    
    const char* forceForward = std::getenv("GE_FORCE_FORWARD");
    if (forceForward && std::string(forceForward) == "1") {
        SetPipeline(RenderPipelineType::Forward);
    } else {
        SetPipeline(RenderPipelineType::Deferred);
    }
    
    Logger::Info("Render Manager initialized successfully");
    return true;
}

void RenderManager::Shutdown() {
    Logger::Info("Shutting down Render Manager");
    
    if (m_deferredPipeline) {
        m_deferredPipeline->Shutdown();
        m_deferredPipeline.reset();
    }
    
    if (m_forwardPipeline) {
        m_forwardPipeline->Shutdown();
        m_forwardPipeline.reset();
    }
    
    if (m_raytracingPipeline) {
        m_raytracingPipeline->Shutdown();
        m_raytracingPipeline.reset();
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
            m_currentPipeline = m_forwardPipeline.get();
            m_currentPipelineType = RenderPipelineType::Forward;
            Logger::Info("Switched to Forward Rendering Pipeline");
            break;
        case RenderPipelineType::Raytracing:
            m_currentPipeline = m_raytracingPipeline.get();
            m_currentPipelineType = RenderPipelineType::Raytracing;
            Logger::Info("Switched to Raytracing Pipeline");
            break;
    }
}

void RenderManager::BeginFrame(const RenderData& renderData) {
    PROFILE_GPU("RenderManager::BeginFrame");
    if (m_currentPipeline) {
        m_currentPipeline->BeginFrame(renderData);
    }
}

void RenderManager::Render(World* world) {
    PROFILE_GPU("RenderManager::Render");
    if (m_currentPipeline) {
        m_currentPipeline->Render(world);
    }
}

void RenderManager::EndFrame() {
    PROFILE_GPU("RenderManager::EndFrame");
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
    
    if (m_forwardPipeline) {
        m_forwardPipeline->Resize(width, height);
    }
    
    if (m_raytracingPipeline) {
        m_raytracingPipeline->Resize(width, height);
    }
}

std::shared_ptr<Texture> RenderManager::GetFinalTexture() const {
    if (m_currentPipeline) {
        return m_currentPipeline->GetFinalTexture();
    }
    return nullptr;
}

}
