#include "ForwardRenderPipeline.h"
#include "../../Core/Logging/Logger.h"
#include "../../Core/ECS/World.h"
#include <GL/gl.h>

namespace GameEngine {

ForwardRenderPipeline::ForwardRenderPipeline() = default;

ForwardRenderPipeline::~ForwardRenderPipeline() {
    Cleanup();
}

bool ForwardRenderPipeline::Initialize(int width, int height) {
    if (m_initialized) {
        return true;
    }
    
    m_renderData.viewportWidth = width;
    m_renderData.viewportHeight = height;
    
    m_framebuffer = std::make_shared<FrameBuffer>(width, height);
    
    m_colorTexture = std::make_shared<Texture>();
    m_colorTexture->CreateEmpty(width, height, TextureFormat::RGBA8);
    
    m_depthTexture = std::make_shared<Texture>();
    m_depthTexture->CreateEmpty(width, height, TextureFormat::Depth24);
    
    m_framebuffer->AddColorAttachment(TextureFormat::RGBA8);
    m_framebuffer->AddDepthAttachment(TextureFormat::Depth24);
    
    if (!m_framebuffer->IsComplete()) {
        Logger::Error("Forward rendering framebuffer is not complete");
        return false;
    }
    
    m_forwardShader = std::make_shared<Shader>();
    m_transparentShader = std::make_shared<Shader>();
    m_effectsShader = std::make_shared<Shader>();
    
    std::string vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec3 aColor;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        out vec3 FragPos;
        out vec3 Normal;
        out vec3 Color;
        
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            Color = aColor;
            
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";
    
    std::string fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 FragPos;
        in vec3 Normal;
        in vec3 Color;
        
        uniform vec3 lightPos;
        uniform vec3 lightColor;
        uniform vec3 viewPos;
        
        void main() {
            vec3 ambient = 0.15 * lightColor;
            
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;
            
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = spec * lightColor;
            
            vec3 result = (ambient + diffuse + specular) * Color;
            FragColor = vec4(result, 1.0);
        }
    )";
    
    m_forwardShader->LoadFromSource(vertexShaderSource, fragmentShaderSource);
    
    std::string transparentFragmentSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 FragPos;
        in vec3 Normal;
        in vec3 Color;
        
        uniform vec3 lightPos;
        uniform vec3 lightColor;
        uniform vec3 viewPos;
        uniform float alpha;
        
        void main() {
            vec3 ambient = 0.15 * lightColor;
            
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;
            
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = spec * lightColor;
            
            vec3 result = (ambient + diffuse + specular) * Color;
            FragColor = vec4(result, alpha);
        }
    )";
    
    m_transparentShader->LoadFromSource(vertexShaderSource, transparentFragmentSource);
    m_effectsShader->LoadFromSource(vertexShaderSource, fragmentShaderSource);
    
    m_initialized = true;
    Logger::Info("Forward rendering pipeline initialized successfully");
    return true;
}

void ForwardRenderPipeline::Render(World* world) {
    if (!m_initialized || !world) {
        return;
    }
    
    m_framebuffer->Bind();
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    
    SetupLighting();
    
    RenderOpaqueObjects(world);
    
    if (m_transparencyEnabled) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        SortTransparentObjects(world);
        RenderTransparentObjects(world);
        
        glDisable(GL_BLEND);
    }
    
    RenderSpecialEffects(world);
    
    m_framebuffer->Unbind();
    
    Logger::Debug("Forward rendering pass completed");
}

void ForwardRenderPipeline::Resize(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }
    
    m_renderData.viewportWidth = width;
    m_renderData.viewportHeight = height;
    
    if (m_initialized) {
        Cleanup();
        Initialize(width, height);
    }
    
    Logger::Info("Forward rendering pipeline resized to " + std::to_string(width) + "x" + std::to_string(height));
}

void ForwardRenderPipeline::Shutdown() {
    Cleanup();
}

void ForwardRenderPipeline::BeginFrame(const RenderData& renderData) {
    m_renderData = renderData;
}

void ForwardRenderPipeline::EndFrame() {
}

std::shared_ptr<Texture> ForwardRenderPipeline::GetFinalTexture() const {
    return m_colorTexture;
}

void ForwardRenderPipeline::Cleanup() {
    m_forwardShader.reset();
    m_transparentShader.reset();
    m_effectsShader.reset();
    m_framebuffer.reset();
    m_colorTexture.reset();
    m_depthTexture.reset();
    
    m_initialized = false;
    Logger::Info("Forward rendering pipeline cleaned up");
}

void ForwardRenderPipeline::RenderOpaqueObjects(World* /*world*/) {
    if (!m_forwardShader) {
        return;
    }
    
    m_forwardShader->Use();
    
    m_forwardShader->SetVector3("lightPos", Vector3(5.0f, 5.0f, 5.0f));
    m_forwardShader->SetVector3("lightColor", Vector3(1.0f, 1.0f, 1.0f));
    m_forwardShader->SetVector3("viewPos", Vector3(0.0f, 0.0f, 3.0f));
    
    Logger::Debug("Rendered opaque objects (simplified for demo)");
}

void ForwardRenderPipeline::RenderTransparentObjects(World* /*world*/) {
    if (!m_transparentShader) {
        return;
    }
    
    m_transparentShader->Use();
    
    m_transparentShader->SetVector3("lightPos", Vector3(5.0f, 5.0f, 5.0f));
    m_transparentShader->SetVector3("lightColor", Vector3(1.0f, 1.0f, 1.0f));
    m_transparentShader->SetVector3("viewPos", Vector3(0.0f, 0.0f, 3.0f));
    m_transparentShader->SetFloat("alpha", 0.7f);
    
    Logger::Debug("Rendered transparent objects (simplified for demo)");
}

void ForwardRenderPipeline::RenderSpecialEffects(World* /*world*/) {
    if (!m_effectsShader) {
        return;
    }
    
    m_effectsShader->Use();
    
    Logger::Debug("Rendered special effects (simplified for demo)");
}

void ForwardRenderPipeline::SetupLighting() {
    Logger::Debug("Forward rendering lighting setup (simplified)");
}

void ForwardRenderPipeline::SortTransparentObjects(World* /*world*/) {
    Logger::Debug("Sorted transparent objects by depth (simplified)");
}

}
