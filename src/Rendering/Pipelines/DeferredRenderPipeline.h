#pragma once

#include "RenderPipeline.h"
#include "../Core/FrameBuffer.h"
#include "../Shaders/Shader.h"
#include <memory>

namespace GameEngine {
    class DeferredRenderPipeline : public RenderPipeline {
    public:
        DeferredRenderPipeline();
        ~DeferredRenderPipeline() override;
        
        bool Initialize(int width, int height) override;
        void Shutdown() override;
        
        void BeginFrame(const RenderData& renderData) override;
        void Render(World* world) override;
        void EndFrame() override;
        
        void Resize(int width, int height) override;
        
        std::shared_ptr<Texture> GetFinalTexture() const override;
        std::shared_ptr<FrameBuffer> GetFramebuffer() const override;

    private:
        std::unique_ptr<FrameBuffer> m_gBuffer;
        std::unique_ptr<FrameBuffer> m_lightingBuffer;
        std::unique_ptr<FrameBuffer> m_shadowMapBuffer;
        
        std::unique_ptr<Shader> m_geometryShader;
        std::unique_ptr<Shader> m_lightingShader;
        std::unique_ptr<Shader> m_compositeShader;
        
        static const int SHADOW_MAP_SIZE = 1024;
        
        void CreateGBuffer();
        void CreateShaders();
        
        void ShadowPass(World* world);
        void GeometryPass(World* world);
        void LightingPass(World* world);
        void CompositePass();
        
        void RenderFullscreenQuad();
    };
}
