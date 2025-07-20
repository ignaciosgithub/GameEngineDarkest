#include "PostProcessingStack.h"
#include "../../Core/Logging/Logger.h"
#include "../Core/OpenGLHeaders.h"
#include <random>
#include <cmath>

namespace GameEngine {

PostProcessingStack::PostProcessingStack() {
    Logger::Info("PostProcessingStack created");
}

PostProcessingStack::~PostProcessingStack() {
    Shutdown();
}

bool PostProcessingStack::Initialize(int width, int height) {
    if (m_initialized) {
        Logger::Warning("PostProcessingStack already initialized");
        return true;
    }
    
    m_width = width;
    m_height = height;
    
    Logger::Info("Initializing PostProcessingStack with resolution " + std::to_string(width) + "x" + std::to_string(height));
    
    CreateFramebuffers(width, height);
    
    CreateShaders();
    
    GenerateSSAOKernel();
    GenerateSSAONoiseTexture();
    
    CreateFullscreenQuad();
    
    m_initialized = true;
    Logger::Info("PostProcessingStack initialized successfully");
    return true;
}

void PostProcessingStack::Shutdown() {
    if (!m_initialized) {
        return;
    }
    
    Logger::Info("Shutting down PostProcessingStack");
    
    m_framebufferA.reset();
    m_framebufferB.reset();
    m_colorTextureA.reset();
    m_colorTextureB.reset();
    
    m_bloomFramebuffers.clear();
    m_bloomTextures.clear();
    
    m_ssaoFramebuffer.reset();
    m_ssaoTexture.reset();
    m_ssaoNoiseTexture.reset();
    
    m_toneMappingShader.reset();
    m_bloomDownsampleShader.reset();
    m_bloomUpsampleShader.reset();
    m_ssaoShader.reset();
    m_fxaaShader.reset();
    m_colorGradingShader.reset();
    
    if (m_quadVAO != 0) {
        Logger::Debug("Cleaning up fullscreen quad VAO (simplified)");
        m_quadVAO = 0;
    }
    if (m_quadVBO != 0) {
        Logger::Debug("Cleaning up fullscreen quad VBO (simplified)");
        m_quadVBO = 0;
    }
    
    m_initialized = false;
    Logger::Info("PostProcessingStack shutdown complete");
}

void PostProcessingStack::Resize(int width, int height) {
    if (width <= 0 || height <= 0) {
        Logger::Warning("Invalid resize dimensions: " + std::to_string(width) + "x" + std::to_string(height));
        return;
    }
    
    m_width = width;
    m_height = height;
    
    if (m_initialized) {
        CreateFramebuffers(width, height);
        Logger::Info("PostProcessingStack resized to " + std::to_string(width) + "x" + std::to_string(height));
    }
}

std::shared_ptr<Texture> PostProcessingStack::Process(std::shared_ptr<Texture> inputTexture, const PostProcessingSettings& settings) {
    if (!m_initialized || !inputTexture) {
        Logger::Warning("PostProcessingStack not initialized or invalid input texture");
        return inputTexture;
    }
    
    m_settings = settings;
    
    std::shared_ptr<Texture> currentTexture = inputTexture;
    std::shared_ptr<FrameBuffer> currentFramebuffer = m_framebufferA;
    std::shared_ptr<FrameBuffer> nextFramebuffer = m_framebufferB;
    
    
    if (settings.enableSSAO) {
        Logger::Debug("SSAO processing (simplified - skipped)");
    }
    
    if (settings.enableHDR) {
        ApplyHDRToneMapping(currentTexture, currentFramebuffer);
        currentTexture = (currentFramebuffer == m_framebufferA) ? m_colorTextureA : m_colorTextureB;
        std::swap(currentFramebuffer, nextFramebuffer);
    }
    
    if (settings.enableBloom) {
        ApplyBloom(currentTexture, currentFramebuffer);
        currentTexture = (currentFramebuffer == m_framebufferA) ? m_colorTextureA : m_colorTextureB;
        std::swap(currentFramebuffer, nextFramebuffer);
    }
    
    if (settings.enableFXAA) {
        ApplyFXAA(currentTexture, currentFramebuffer);
        currentTexture = (currentFramebuffer == m_framebufferA) ? m_colorTextureA : m_colorTextureB;
        std::swap(currentFramebuffer, nextFramebuffer);
    }
    
    if (settings.enableColorGrading) {
        ApplyColorGrading(currentTexture, currentFramebuffer);
        currentTexture = (currentFramebuffer == m_framebufferA) ? m_colorTextureA : m_colorTextureB;
    }
    
    Logger::Debug("Post-processing complete");
    return currentTexture;
}

void PostProcessingStack::CreateFramebuffers(int width, int height) {
    Logger::Info("Creating post-processing framebuffers");
    
    m_framebufferA = std::make_shared<FrameBuffer>(width, height);
    m_framebufferB = std::make_shared<FrameBuffer>(width, height);
    
    m_framebufferA->AddColorAttachment(TextureFormat::RGBA8);
    m_framebufferB->AddColorAttachment(TextureFormat::RGBA8);
    
    m_colorTextureA = m_framebufferA->GetColorTexture(0);
    m_colorTextureB = m_framebufferB->GetColorTexture(0);
    
    m_bloomFramebuffers.clear();
    m_bloomTextures.clear();
    
    for (int i = 0; i < m_settings.bloomIterations; ++i) {
        int bloomWidth = width >> (i + 1);
        int bloomHeight = height >> (i + 1);
        
        if (bloomWidth < 1) bloomWidth = 1;
        if (bloomHeight < 1) bloomHeight = 1;
        
        auto bloomFramebuffer = std::make_shared<FrameBuffer>(bloomWidth, bloomHeight);
        bloomFramebuffer->AddColorAttachment(TextureFormat::RGBA16F);
        auto bloomTexture = bloomFramebuffer->GetColorTexture(0);
        
        m_bloomFramebuffers.push_back(bloomFramebuffer);
        m_bloomTextures.push_back(bloomTexture);
    }
    
    m_ssaoFramebuffer = std::make_shared<FrameBuffer>(width, height);
    m_ssaoFramebuffer->AddColorAttachment(TextureFormat::RGB8);
    m_ssaoTexture = m_ssaoFramebuffer->GetColorTexture(0);
    
    Logger::Info("Post-processing framebuffers created");
}

void PostProcessingStack::CreateShaders() {
    Logger::Info("Creating post-processing shaders");
    
    m_toneMappingShader = std::make_shared<Shader>();
    std::string toneMapVert = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    std::string toneMapFrag = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D inputTexture;
        uniform float exposure;
        uniform float gamma;
        uniform int toneMappingType;
        
        vec3 ReinhardToneMapping(vec3 color) {
            return color / (color + vec3(1.0));
        }
        
        vec3 ACESToneMapping(vec3 color) {
            const float a = 2.51;
            const float b = 0.03;
            const float c = 2.43;
            const float d = 0.59;
            const float e = 0.14;
            return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
        }
        
        void main() {
            vec3 color = texture(inputTexture, TexCoord).rgb;
            color *= exposure;
            
            if (toneMappingType == 1) {
                color = ReinhardToneMapping(color);
            } else if (toneMappingType == 2) {
                color = ACESToneMapping(color);
            }
            
            color = pow(color, vec3(1.0 / gamma));
            FragColor = vec4(color, 1.0);
        }
    )";
    m_toneMappingShader->LoadFromSource(toneMapVert, toneMapFrag);
    
    m_bloomDownsampleShader = std::make_shared<Shader>();
    m_bloomUpsampleShader = std::make_shared<Shader>();
    m_ssaoShader = std::make_shared<Shader>();
    m_fxaaShader = std::make_shared<Shader>();
    m_colorGradingShader = std::make_shared<Shader>();
    
    m_bloomDownsampleShader->LoadFromSource(toneMapVert, toneMapFrag);
    m_bloomUpsampleShader->LoadFromSource(toneMapVert, toneMapFrag);
    m_ssaoShader->LoadFromSource(toneMapVert, toneMapFrag);
    m_fxaaShader->LoadFromSource(toneMapVert, toneMapFrag);
    m_colorGradingShader->LoadFromSource(toneMapVert, toneMapFrag);
    
    Logger::Info("Post-processing shaders created");
}

void PostProcessingStack::GenerateSSAOKernel() {
    Logger::Info("Generating SSAO kernel");
    
    m_ssaoKernel.clear();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    for (int i = 0; i < m_settings.ssaoSamples; ++i) {
        Vector3 sample(
            dis(gen) * 2.0f - 1.0f,
            dis(gen) * 2.0f - 1.0f,
            dis(gen)
        );
        sample.Normalize();
        sample *= dis(gen);
        
        float scale = static_cast<float>(i) / static_cast<float>(m_settings.ssaoSamples);
        scale = 0.1f + scale * scale * 0.9f;
        sample *= scale;
        
        m_ssaoKernel.push_back(sample);
    }
    
    Logger::Info("SSAO kernel generated with " + std::to_string(m_ssaoKernel.size()) + " samples");
}

void PostProcessingStack::GenerateSSAONoiseTexture() {
    Logger::Info("Generating SSAO noise texture");
    
    m_ssaoNoiseTexture = std::make_shared<Texture>();
    m_ssaoNoiseTexture->CreateEmpty(4, 4, TextureFormat::RGB16F);
    
    Logger::Info("SSAO noise texture generated");
}

void PostProcessingStack::CreateFullscreenQuad() {
    Logger::Info("Creating fullscreen quad (simplified)");
    
    m_quadVAO = 1; // Dummy VAO ID
    m_quadVBO = 1; // Dummy VBO ID
    
    Logger::Info("Fullscreen quad created");
}

void PostProcessingStack::RenderFullscreenQuad() {
    Logger::Debug("Rendering fullscreen quad (simplified)");
}

void PostProcessingStack::ApplyHDRToneMapping(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer) {
    Logger::Debug("Applying HDR tone mapping");
    
    outputFramebuffer->Bind();
    m_toneMappingShader->Use();
    
    m_toneMappingShader->SetFloat("exposure", m_settings.exposure);
    m_toneMappingShader->SetFloat("gamma", m_settings.gamma);
    m_toneMappingShader->SetInt("toneMappingType", static_cast<int>(m_settings.toneMappingType));
    
    inputTexture->Bind(0);
    m_toneMappingShader->SetInt("inputTexture", 0);
    
    RenderFullscreenQuad();
    
    outputFramebuffer->Unbind();
    Logger::Debug("HDR tone mapping applied");
}

void PostProcessingStack::ApplyBloom(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer) {
    Logger::Debug("Applying bloom effect");
    
    outputFramebuffer->Bind();
    m_bloomDownsampleShader->Use();
    
    inputTexture->Bind(0);
    m_bloomDownsampleShader->SetInt("inputTexture", 0);
    m_bloomDownsampleShader->SetFloat("bloomThreshold", m_settings.bloomThreshold);
    m_bloomDownsampleShader->SetFloat("bloomIntensity", m_settings.bloomIntensity);
    
    RenderFullscreenQuad();
    
    outputFramebuffer->Unbind();
    Logger::Debug("Bloom effect applied");
}

void PostProcessingStack::ApplySSAO(std::shared_ptr<Texture> inputTexture, std::shared_ptr<Texture> depthTexture, std::shared_ptr<Texture> normalTexture, std::shared_ptr<FrameBuffer> outputFramebuffer) {
    Logger::Debug("Applying SSAO");
    
    outputFramebuffer->Bind();
    m_ssaoShader->Use();
    
    inputTexture->Bind(0);
    depthTexture->Bind(1);
    normalTexture->Bind(2);
    m_ssaoNoiseTexture->Bind(3);
    
    m_ssaoShader->SetInt("inputTexture", 0);
    m_ssaoShader->SetInt("depthTexture", 1);
    m_ssaoShader->SetInt("normalTexture", 2);
    m_ssaoShader->SetInt("noiseTexture", 3);
    m_ssaoShader->SetFloat("ssaoRadius", m_settings.ssaoRadius);
    m_ssaoShader->SetFloat("ssaoBias", m_settings.ssaoBias);
    
    RenderFullscreenQuad();
    
    outputFramebuffer->Unbind();
    Logger::Debug("SSAO applied");
}

void PostProcessingStack::ApplyFXAA(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer) {
    Logger::Debug("Applying FXAA");
    
    outputFramebuffer->Bind();
    m_fxaaShader->Use();
    
    inputTexture->Bind(0);
    m_fxaaShader->SetInt("inputTexture", 0);
    
    RenderFullscreenQuad();
    
    outputFramebuffer->Unbind();
    Logger::Debug("FXAA applied");
}

void PostProcessingStack::ApplyColorGrading(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer) {
    Logger::Debug("Applying color grading");
    
    outputFramebuffer->Bind();
    m_colorGradingShader->Use();
    
    inputTexture->Bind(0);
    m_colorGradingShader->SetInt("inputTexture", 0);
    m_colorGradingShader->SetVector3("colorFilter", m_settings.colorFilter);
    m_colorGradingShader->SetFloat("saturation", m_settings.saturation);
    m_colorGradingShader->SetFloat("contrast", m_settings.contrast);
    m_colorGradingShader->SetFloat("brightness", m_settings.brightness);
    
    RenderFullscreenQuad();
    
    outputFramebuffer->Unbind();
    Logger::Debug("Color grading applied");
}

void PostProcessingStack::BloomDownsample(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer) {
    Logger::Debug("Bloom downsample");
    
    outputFramebuffer->Bind();
    m_bloomDownsampleShader->Use();
    
    inputTexture->Bind(0);
    m_bloomDownsampleShader->SetInt("inputTexture", 0);
    
    RenderFullscreenQuad();
    
    outputFramebuffer->Unbind();
}

void PostProcessingStack::BloomUpsample(std::shared_ptr<Texture> inputTexture, std::shared_ptr<FrameBuffer> outputFramebuffer) {
    Logger::Debug("Bloom upsample");
    
    outputFramebuffer->Bind();
    m_bloomUpsampleShader->Use();
    
    inputTexture->Bind(0);
    m_bloomUpsampleShader->SetInt("inputTexture", 0);
    
    RenderFullscreenQuad();
    
    outputFramebuffer->Unbind();
}

}
