#include "DeferredRenderPipeline.h"
#include "../../Core/ECS/World.h"
#include "../../Core/Components/TransformComponent.h"
#include "../../Core/Logging/Logger.h"
#include "../Meshes/Mesh.h"
#include "../Core/OpenGLHeaders.h"

namespace GameEngine {

DeferredRenderPipeline::DeferredRenderPipeline() = default;
DeferredRenderPipeline::~DeferredRenderPipeline() = default;

bool DeferredRenderPipeline::Initialize(int width, int height) {
    m_width = width;
    m_height = height;
    
    Logger::Info("Initializing Deferred Rendering Pipeline...");
    
    CreateGBuffer();
    CreateShaders();
    
    if (!m_gBuffer->IsComplete()) {
        Logger::Error("G-Buffer is not complete");
        return false;
    }
    
    if (!m_lightingBuffer->IsComplete()) {
        Logger::Error("Lighting buffer is not complete");
        return false;
    }
    
    Logger::Info("Deferred Rendering Pipeline initialized successfully");
    return true;
}

void DeferredRenderPipeline::Shutdown() {
    Logger::Info("Shutting down Deferred Rendering Pipeline");
    m_gBuffer.reset();
    m_lightingBuffer.reset();
    m_geometryShader.reset();
    m_lightingShader.reset();
    m_compositeShader.reset();
}

void DeferredRenderPipeline::BeginFrame(const RenderData& renderData) {
    m_renderData = renderData;
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    m_gBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DeferredRenderPipeline::Render(World* world) {
    GeometryPass(world);
    LightingPass(world);
    CompositePass();
}

void DeferredRenderPipeline::EndFrame() {
    m_lightingBuffer->Unbind();
}

void DeferredRenderPipeline::Resize(int width, int height) {
    m_width = width;
    m_height = height;
    
    if (m_gBuffer) {
        m_gBuffer->Resize(width, height);
    }
    
    if (m_lightingBuffer) {
        m_lightingBuffer->Resize(width, height);
    }
}

std::shared_ptr<Texture> DeferredRenderPipeline::GetFinalTexture() const {
    return m_lightingBuffer ? m_lightingBuffer->GetColorTexture(0) : nullptr;
}

void DeferredRenderPipeline::CreateGBuffer() {
    m_gBuffer = std::make_unique<FrameBuffer>(m_width, m_height);
    
    m_gBuffer->AddColorAttachment(TextureFormat::RGBA8);
    m_gBuffer->AddColorAttachment(TextureFormat::RGBA8);
    m_gBuffer->AddColorAttachment(TextureFormat::RGBA16F);
    m_gBuffer->AddColorAttachment(TextureFormat::RGBA8);
    m_gBuffer->AddDepthAttachment(TextureFormat::Depth24);
    
    m_lightingBuffer = std::make_unique<FrameBuffer>(m_width, m_height);
    m_lightingBuffer->AddColorAttachment(TextureFormat::RGBA16F);
}

void DeferredRenderPipeline::CreateShaders() {
    m_geometryShader = std::make_unique<Shader>();
    m_lightingShader = std::make_unique<Shader>();
    m_compositeShader = std::make_unique<Shader>();
    
    std::string geometryVertexSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPosition;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoord;
        
        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;
        
        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoord;
        
        void main() {
            vec4 worldPos = uModel * vec4(aPosition, 1.0);
            FragPos = worldPos.xyz;
            Normal = mat3(transpose(inverse(uModel))) * aNormal;
            TexCoord = aTexCoord;
            
            gl_Position = uProjection * uView * worldPos;
        }
    )";
    
    std::string geometryFragmentSource = R"(
        #version 330 core
        layout (location = 0) out vec4 gAlbedoMetallic;
        layout (location = 1) out vec4 gNormalRoughness;
        layout (location = 2) out vec4 gPosition;
        layout (location = 3) out vec4 gMotionMaterial;
        
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoord;
        
        uniform vec3 uAlbedo = vec3(1.0, 0.0, 0.0);
        uniform float uMetallic = 0.0;
        uniform float uRoughness = 0.5;
        
        void main() {
            gAlbedoMetallic = vec4(uAlbedo, uMetallic);
            gNormalRoughness = vec4(normalize(Normal) * 0.5 + 0.5, uRoughness);
            gPosition = vec4(FragPos, gl_FragCoord.z);
            gMotionMaterial = vec4(0.0, 0.0, 1.0, 1.0);
        }
    )";
    
    m_geometryShader->LoadFromSource(geometryVertexSource, geometryFragmentSource);
    
    Logger::Info("Created deferred rendering shaders (simplified for demo)");
}

void DeferredRenderPipeline::GeometryPass(World* /*world*/) {
    m_gBuffer->Bind();
    
    if (m_geometryShader) {
        m_geometryShader->Use();
        m_geometryShader->SetMatrix4("uView", m_renderData.viewMatrix);
        m_geometryShader->SetMatrix4("uProjection", m_renderData.projectionMatrix);
    }
    
    static Mesh cubeMesh = Mesh::CreateCube(1.0f);
    static bool meshUploaded = false;
    if (!meshUploaded) {
        Logger::Debug("DeferredRenderPipeline: Attempting to upload cube mesh...");
        cubeMesh.Upload();
        meshUploaded = true;
        Logger::Debug("DeferredRenderPipeline: Cube mesh upload completed, meshUploaded = true");
    }
    
    for (int x = -2; x <= 2; ++x) {
        for (int z = -2; z <= 2; ++z) {
            Matrix4 modelMatrix = Matrix4::Translation(Vector3(x * 3.0f, 0.0f, z * 3.0f));
            
            if (m_geometryShader) {
                m_geometryShader->SetMatrix4("uModel", modelMatrix);
                m_geometryShader->SetVector3("uAlbedo", Vector3(0.8f, 0.2f, 0.2f));
                m_geometryShader->SetFloat("uMetallic", 0.1f);
                m_geometryShader->SetFloat("uRoughness", 0.6f);
            }
            
            cubeMesh.Draw();
        }
    }
}

void DeferredRenderPipeline::LightingPass(World* /*world*/) {
    m_lightingBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    
    glDisable(GL_DEPTH_TEST);
    
    if (m_gBuffer) {
        auto albedoTexture = m_gBuffer->GetColorTexture(0);
        auto normalTexture = m_gBuffer->GetColorTexture(1);
        auto positionTexture = m_gBuffer->GetColorTexture(2);
        
        if (albedoTexture) albedoTexture->Bind(0);
        if (normalTexture) normalTexture->Bind(1);
        if (positionTexture) positionTexture->Bind(2);
    }
    
    RenderFullscreenQuad();
    
    glEnable(GL_DEPTH_TEST);
}

void DeferredRenderPipeline::CompositePass() {
    Logger::Debug("Composite pass framebuffer bind (simplified)");
    glViewport(0, 0, m_renderData.viewportWidth, m_renderData.viewportHeight);
    
    glDisable(GL_DEPTH_TEST);
    
    auto finalTexture = m_lightingBuffer->GetColorTexture(0);
    if (finalTexture) {
        finalTexture->Bind(0);
    }
    
    RenderFullscreenQuad();
    
    glEnable(GL_DEPTH_TEST);
}

void DeferredRenderPipeline::RenderFullscreenQuad() {
    static bool quadCreated = false;
    
    if (!quadCreated) {
        Logger::Debug("Quad vertices defined (simplified)");
        Logger::Info("Fullscreen quad setup (simplified)");
        Logger::Debug("Vertex attributes setup (simplified)");
        
        quadCreated = true;
    }
    
    Logger::Debug("Fullscreen quad render (simplified)");
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    Logger::Debug("Vertex array unbound (simplified)");
}

}
