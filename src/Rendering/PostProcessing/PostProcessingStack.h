#pragma once

#include "../Core/FrameBuffer.h"
#include "../Core/Texture.h"
#include "../Shaders/Shader.h"
#include "../../Core/Math/Vector3.h"
#include <memory>
#include <vector>

namespace GameEngine {

enum class ToneMappingType {
    None,
    Reinhard,
    ACES,
    Filmic
};

struct PostProcessingSettings {
    // HDR and Tone Mapping
    bool enableHDR = true;
    ToneMappingType toneMappingType = ToneMappingType::ACES;
    float exposure = 1.0f;
    float gamma = 2.2f;
    
    // Bloom
    bool enableBloom = true;
    float bloomThreshold = 1.0f;
    float bloomIntensity = 0.8f;
    int bloomIterations = 5;
    
    // SSAO
    bool enableSSAO = true;
    float ssaoRadius = 0.5f;
    float ssaoBias = 0.025f;
    int ssaoSamples = 64;
    
    // Anti-aliasing
    bool enableFXAA = true;
    
    // Color Grading
    bool enableColorGrading = true;
    Vector3 colorFilter = Vector3(1.0f, 1.0f, 1.0f);
    float saturation = 1.0f;
    float contrast = 1.0f;
    float brightness = 0.0f;
};

class PostProcessingEffect {
public:
    virtual ~PostProcessingEffect() = default;
    virtual bool Initialize(int width, int height) = 0;
    virtual void Render(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer) = 0;
    virtual void Resize(int width, int height) = 0;
    virtual void Cleanup() = 0;
};

class PostProcessingStack {
public:
    PostProcessingStack();
    ~PostProcessingStack();
    
    bool Initialize(int width, int height);
    void Shutdown();
    void Resize(int width, int height);
    
    std::shared_ptr<Texture> Process(std::shared_ptr<Texture> inputTexture, const PostProcessingSettings& settings);
    
    void SetSettings(const PostProcessingSettings& settings) { m_settings = settings; }
    const PostProcessingSettings& GetSettings() const { return m_settings; }
    
private:
    void CreateFramebuffers(int width, int height);
    void CreateShaders();
    void RenderFullscreenQuad();
    
    // Post-processing effects
    void ApplyHDRToneMapping(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer);
    void ApplyBloom(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer);
    void ApplySSAO(std::shared_ptr<Texture> inputTexture, std::shared_ptr<Texture> depthTexture, std::shared_ptr<Texture> normalTexture, std::shared_ptr<FrameBuffer> outputFramebuffer);
    void ApplyFXAA(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer);
    void ApplyColorGrading(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer);
    
    // Bloom helpers
    void BloomDownsample(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer);
    void BloomUpsample(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer);
    
    // Helper methods
    void GenerateSSAOKernel();
    void GenerateSSAONoiseTexture();
    void CreateFullscreenQuad();
    
    PostProcessingSettings m_settings;
    
    // Framebuffers for ping-pong rendering
    std::shared_ptr<FrameBuffer> m_framebufferA;
    std::shared_ptr<FrameBuffer> m_framebufferB;
    std::shared_ptr<Texture> m_colorTextureA;
    std::shared_ptr<Texture> m_colorTextureB;
    
    // Bloom framebuffers (multiple resolutions)
    std::vector<std::shared_ptr<FrameBuffer>> m_bloomFramebuffers;
    std::vector<std::shared_ptr<Texture>> m_bloomTextures;
    
    // SSAO framebuffer
    std::shared_ptr<FrameBuffer> m_ssaoFramebuffer;
    std::shared_ptr<Texture> m_ssaoTexture;
    std::shared_ptr<Texture> m_ssaoNoiseTexture;
    std::vector<Vector3> m_ssaoKernel;
    
    // Shaders
    std::shared_ptr<Shader> m_toneMappingShader;
    std::shared_ptr<Shader> m_bloomDownsampleShader;
    std::shared_ptr<Shader> m_bloomUpsampleShader;
    std::shared_ptr<Shader> m_ssaoShader;
    std::shared_ptr<Shader> m_fxaaShader;
    std::shared_ptr<Shader> m_colorGradingShader;
    
    // Fullscreen quad VAO
    unsigned int m_quadVAO = 0;
    unsigned int m_quadVBO = 0;
    
    int m_width = 0;
    int m_height = 0;
    bool m_initialized = false;
};

}
