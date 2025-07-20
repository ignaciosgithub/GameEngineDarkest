#pragma once

#include "Pipelines/RenderPipeline.h"
#include "Pipelines/DeferredRenderPipeline.h"
#include "Pipelines/ForwardRenderPipeline.h"
#include "Pipelines/RaytracingPipeline.h"
#include <memory>

namespace GameEngine {
    enum class RenderPipelineType {
        Deferred,
        Forward,
        Raytracing
    };

    class RenderManager {
    public:
        RenderManager();
        ~RenderManager();

        bool Initialize(int width, int height);
        void Shutdown();

        void SetPipeline(RenderPipelineType type);
        RenderPipelineType GetCurrentPipeline() const { return m_currentPipelineType; }

        void BeginFrame(const RenderData& renderData);
        void Render(World* world);
        void EndFrame();

        void Resize(int width, int height);
        
        std::shared_ptr<Texture> GetFinalTexture() const;

    private:
        std::unique_ptr<DeferredRenderPipeline> m_deferredPipeline;
        std::unique_ptr<ForwardRenderPipeline> m_forwardPipeline;
        std::unique_ptr<RaytracingPipeline> m_raytracingPipeline;
        RenderPipeline* m_currentPipeline = nullptr;
        RenderPipelineType m_currentPipelineType = RenderPipelineType::Deferred;
        
        int m_width = 0;
        int m_height = 0;
    };
}
