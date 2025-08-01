#include "DeferredRenderPipeline.h"
#include "../../Core/ECS/World.h"
#include "../../Core/Components/TransformComponent.h"
#include "../../Core/Logging/Logger.h"
#include "../Meshes/Mesh.h"
#include "../Core/OpenGLHeaders.h"
#include "../Lighting/LightManager.h"
#include "../Lighting/Light.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996) // Disable sprintf deprecation warning
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#include "../Core/stb_image_write.h"
#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include <string>
#include <cstring>

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
    
    static int frameCount = 0;
    std::string filename = "frames/frame" + std::to_string(frameCount) + ".png";
    
    Logger::Info("Saving frame " + std::to_string(frameCount) + " to " + filename);
    
    unsigned char* pixels = new unsigned char[m_width * m_height * 4];
    glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    unsigned char* flippedPixels = new unsigned char[m_width * m_height * 4];
    for (int y = 0; y < m_height; y++) {
        std::memcpy(flippedPixels + (m_height - 1 - y) * m_width * 4, 
                   pixels + y * m_width * 4, 
                   m_width * 4);
    }
    
    int result = stbi_write_png(filename.c_str(), m_width, m_height, 4, flippedPixels, m_width * 4);
    if (result) {
        Logger::Info("Successfully saved frame " + std::to_string(frameCount));
    } else {
        Logger::Error("Failed to save frame " + std::to_string(frameCount));
    }
    
    delete[] pixels;
    delete[] flippedPixels;
    
    frameCount++;
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
        layout (location = 2) in vec3 aColor;
        
        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;
        
        out vec3 FragPos;
        out vec3 Normal;
        flat out vec3 VertexColor;
        
        void main() {
            vec4 worldPos = uModel * vec4(aPosition, 1.0);
            FragPos = worldPos.xyz;
            Normal = mat3(transpose(inverse(uModel))) * aNormal;
            VertexColor = aColor;
            
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
        flat in vec3 VertexColor;
        
        uniform float uMetallic = 0.0;
        uniform float uRoughness = 0.5;
        
        void main() {
            gAlbedoMetallic = vec4(VertexColor, uMetallic);
            gNormalRoughness = vec4(normalize(Normal) * 0.5 + 0.5, uRoughness);
            gPosition = vec4(FragPos, gl_FragCoord.z);
            gMotionMaterial = vec4(0.0, 0.0, 1.0, 1.0);
        }
    )";
    
    m_geometryShader->LoadFromSource(geometryVertexSource, geometryFragmentSource);
    
    std::string lightingVertexSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPosition;
        layout (location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;
        
        void main() {
            TexCoord = aTexCoord;
            gl_Position = vec4(aPosition, 0.0, 1.0);
        }
    )";
    
    std::string lightingFragmentSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        
        uniform sampler2D gAlbedoMetallic;
        uniform sampler2D gNormalRoughness;
        uniform sampler2D gPosition;
        
        uniform int numLights;
        uniform vec3 lightPositions[32];
        uniform vec3 lightColors[32];
        uniform float lightIntensities[32];
        uniform int lightTypes[32];
        uniform float lightRanges[32];
        uniform vec3 viewPos;
        
        void main() {
            vec4 albedoMetallic = texture(gAlbedoMetallic, TexCoord);
            vec4 normalRoughness = texture(gNormalRoughness, TexCoord);
            vec4 position = texture(gPosition, TexCoord);
            
            if (position.w <= 0.0 || length(albedoMetallic.rgb) < 0.01) {
                FragColor = vec4(0.0, 0.0, 0.0, 1.0);
                return;
            }
            
            vec3 albedo = albedoMetallic.rgb;
            vec3 normal = normalize(normalRoughness.rgb * 2.0 - 1.0);
            vec3 fragPos = position.xyz;
            vec3 viewDir = normalize(viewPos - fragPos);
            
            vec3 totalLighting = vec3(0.0);
            float totalBrightness = 0.0;
            
            for(int i = 0; i < numLights && i < 32; i++) {
                vec3 lightContribution = vec3(0.0);
                
                if(lightTypes[i] == 0) {
                    vec3 lightDir = normalize(-lightPositions[i]);
                    float diff = max(dot(normal, lightDir), 0.0);
                    vec3 diffuse = diff * lightColors[i] * lightIntensities[i] * albedo;
                    
                    vec3 reflectDir = reflect(-lightDir, normal);
                    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
                    vec3 specular = spec * lightColors[i] * lightIntensities[i];
                    
                    lightContribution = diffuse + specular;
                } else if(lightTypes[i] == 1) {
                    vec3 lightDir = normalize(lightPositions[i] - fragPos);
                    float distance = length(lightPositions[i] - fragPos);
                    
                    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
                    if(distance > lightRanges[i]) attenuation = 0.0;
                    
                    float diff = max(dot(normal, lightDir), 0.0);
                    vec3 diffuse = diff * lightColors[i] * lightIntensities[i] * attenuation * albedo;
                    
                    vec3 reflectDir = reflect(-lightDir, normal);
                    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
                    vec3 specular = spec * lightColors[i] * lightIntensities[i] * attenuation;
                    
                    lightContribution = diffuse + specular;
                }
                
                totalLighting += lightContribution;
                totalBrightness += lightIntensities[i];
            }
            
            if(totalBrightness > 100.0) {
                totalLighting *= (100.0 / totalBrightness);
            }
            
            vec3 ambient = vec3(0.1, 0.1, 0.1) * albedo;
            vec3 result = ambient + totalLighting;
            
            FragColor = vec4(result, 1.0);
        }
    )";
    
    m_lightingShader->LoadFromSource(lightingVertexSource, lightingFragmentSource);
    
    std::string compositeVertexSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPosition;
        layout (location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;
        
        void main() {
            TexCoord = aTexCoord;
            gl_Position = vec4(aPosition, 0.0, 1.0);
        }
    )";
    
    std::string compositeFragmentSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        
        uniform sampler2D finalTexture;
        
        void main() {
            FragColor = texture(finalTexture, TexCoord);
        }
    )";
    
    m_compositeShader->LoadFromSource(compositeVertexSource, compositeFragmentSource);
    
    Logger::Info("Created complete deferred rendering shaders with lighting and composite passes");
}

void DeferredRenderPipeline::GeometryPass(World* world) {
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
    
    if (world) {
        int entityCount = 0;
        for (const auto& entity : world->GetEntities()) {
            if (world->HasComponent<TransformComponent>(entity)) {
                auto* transformComp = world->GetComponent<TransformComponent>(entity);
                if (transformComp) {
                    Matrix4 modelMatrix = transformComp->transform.GetLocalToWorldMatrix();
                    
                    if (m_geometryShader) {
                        m_geometryShader->SetMatrix4("uModel", modelMatrix);
                        m_geometryShader->SetFloat("uMetallic", 0.1f);
                        m_geometryShader->SetFloat("uRoughness", 0.6f);
                    }
                    
                    cubeMesh.Draw();
                    entityCount++;
                }
            }
        }
        Logger::Debug("DeferredRenderPipeline: Rendered " + std::to_string(entityCount) + " entities from World");
    } else {
        Logger::Warning("DeferredRenderPipeline: World is null, falling back to hardcoded cubes");
        for (int x = -2; x <= 2; ++x) {
            for (int z = -2; z <= 2; ++z) {
                Matrix4 modelMatrix = Matrix4::Translation(Vector3(x * 3.0f, 0.0f, z * 3.0f));
                
                if (m_geometryShader) {
                    m_geometryShader->SetMatrix4("uModel", modelMatrix);
                    m_geometryShader->SetFloat("uMetallic", 0.1f);
                    m_geometryShader->SetFloat("uRoughness", 0.6f);
                }
                
                cubeMesh.Draw();
            }
        }
    }
}

void DeferredRenderPipeline::LightingPass(World* world) {
    m_lightingBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    
    glDisable(GL_DEPTH_TEST);
    
    if (m_lightingShader) {
        m_lightingShader->Use();
        
        if (m_gBuffer) {
            auto albedoTexture = m_gBuffer->GetColorTexture(0);
            auto normalTexture = m_gBuffer->GetColorTexture(1);
            auto positionTexture = m_gBuffer->GetColorTexture(2);
            
            if (albedoTexture) {
                albedoTexture->Bind(0);
                m_lightingShader->SetInt("gAlbedoMetallic", 0);
            }
            if (normalTexture) {
                normalTexture->Bind(1);
                m_lightingShader->SetInt("gNormalRoughness", 1);
            }
            if (positionTexture) {
                positionTexture->Bind(2);
                m_lightingShader->SetInt("gPosition", 2);
            }
        }
        
        LightManager lightManager;
        lightManager.CollectLights(world);
        lightManager.ApplyBrightnessLimits();
        
        std::vector<LightManager::ShaderLightData> lightData;
        lightManager.GetShaderLightData(lightData);
        
        m_lightingShader->SetInt("numLights", static_cast<int>(lightData.size()));
        
        for (size_t i = 0; i < lightData.size() && i < MAX_LIGHTS; ++i) {
            std::string indexStr = std::to_string(i);
            m_lightingShader->SetVector3("lightPositions[" + indexStr + "]", lightData[i].position);
            m_lightingShader->SetVector3("lightColors[" + indexStr + "]", lightData[i].color);
            m_lightingShader->SetFloat("lightIntensities[" + indexStr + "]", lightData[i].intensity);
            m_lightingShader->SetInt("lightTypes[" + indexStr + "]", lightData[i].type);
            m_lightingShader->SetFloat("lightRanges[" + indexStr + "]", lightData[i].range);
        }
        
        Matrix4 invViewMatrix = m_renderData.viewMatrix.Inverted();
        Vector3 cameraPosition = Vector3(invViewMatrix.m[12], invViewMatrix.m[13], invViewMatrix.m[14]);
        m_lightingShader->SetVector3("viewPos", cameraPosition);
    }
    
    RenderFullscreenQuad();
    
    glEnable(GL_DEPTH_TEST);
}

void DeferredRenderPipeline::CompositePass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_renderData.viewportWidth, m_renderData.viewportHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glDisable(GL_DEPTH_TEST);
    
    if (m_compositeShader) {
        m_compositeShader->Use();
        
        auto finalTexture = m_lightingBuffer->GetColorTexture(0);
        if (finalTexture) {
            finalTexture->Bind(0);
            m_compositeShader->SetInt("finalTexture", 0);
        }
    }
    
    RenderFullscreenQuad();
    
    glEnable(GL_DEPTH_TEST);
}

void DeferredRenderPipeline::RenderFullscreenQuad() {
    static unsigned int quadVAO = 0;
    static unsigned int quadVBO = 0;
    
    if (quadVAO == 0) {
        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
        };
        
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        
        Logger::Info("Fullscreen quad VAO created successfully");
    }
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

}
