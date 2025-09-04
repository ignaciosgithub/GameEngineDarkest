#pragma once

#include "RenderPipeline.h"
#include "../Core/Texture.h"
#include "../Core/FrameBuffer.h"
#include "../Shaders/Shader.h"
#include <memory>
#include <vector>

namespace GameEngine {

class LightOcclusion;

class ForwardRenderPipeline : public RenderPipeline {
public:
    ForwardRenderPipeline();
    ~ForwardRenderPipeline() override;

    bool Initialize(int width, int height) override;
    void Shutdown() override;
    void BeginFrame(const RenderData& renderData) override;
    void Render(World* world) override;
    void EndFrame() override;
    void Resize(int width, int height) override;
    
    std::shared_ptr<Texture> GetFinalTexture() const override;
    std::shared_ptr<FrameBuffer> GetFramebuffer() const override;

    void SetTransparencyEnabled(bool enabled) { m_transparencyEnabled = enabled; }
    bool IsTransparencyEnabled() const { return m_transparencyEnabled; }

private:
    void Cleanup();

    void RenderOpaqueObjects(World* world);
    void RenderTransparentObjects(World* world);
    void RenderSpecialEffects(World* world);
    void SetupLighting();
    void SortTransparentObjects(World* world);
    void CompositePass();
    void RenderFullscreenQuad();

    void RenderShadowPass(World* world);

    std::shared_ptr<Shader> m_forwardShader;
    std::shared_ptr<Shader> m_transparentShader;
    std::shared_ptr<Shader> m_effectsShader;
    std::shared_ptr<Shader> m_compositeShader;
    std::shared_ptr<Shader> m_depthShader;
    
    std::shared_ptr<FrameBuffer> m_framebuffer;
    std::shared_ptr<Texture> m_colorTexture;
    std::shared_ptr<Texture> m_depthTexture;
    
    bool m_transparencyEnabled = true;
    bool m_initialized = false;

    // Shadow volume SSBOs and occlusion builder
    unsigned int m_shadowVolumeHeadersSSBO = 0;
    unsigned int m_shadowVolumeVerticesSSBO = 0;
    int m_numVolumeHeadersLast = 0;
    std::unique_ptr<LightOcclusion> m_lightOcclusion;
};

}
