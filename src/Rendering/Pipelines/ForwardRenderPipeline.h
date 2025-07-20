#pragma once

#include "RenderPipeline.h"
#include "../Core/Texture.h"
#include "../Core/FrameBuffer.h"
#include "../Shaders/Shader.h"
#include <memory>

namespace GameEngine {

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

    void SetTransparencyEnabled(bool enabled) { m_transparencyEnabled = enabled; }
    bool IsTransparencyEnabled() const { return m_transparencyEnabled; }

private:
    void Cleanup();

    void RenderOpaqueObjects(World* world);
    void RenderTransparentObjects(World* world);
    void RenderSpecialEffects(World* world);
    void SetupLighting();
    void SortTransparentObjects(World* world);

    std::shared_ptr<Shader> m_forwardShader;
    std::shared_ptr<Shader> m_transparentShader;
    std::shared_ptr<Shader> m_effectsShader;
    
    std::shared_ptr<FrameBuffer> m_framebuffer;
    std::shared_ptr<Texture> m_colorTexture;
    std::shared_ptr<Texture> m_depthTexture;
    
    bool m_transparencyEnabled = true;
    bool m_initialized = false;
};

}
