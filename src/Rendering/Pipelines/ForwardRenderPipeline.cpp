#include "ForwardRenderPipeline.h"
#include "../../Core/Logging/Logger.h"
#include "../../Core/ECS/World.h"
#include "../../Core/Components/TransformComponent.h"
#include "../Meshes/Mesh.h"
#include "../Core/OpenGLHeaders.h"
#include "../Core/stb_image_write.h"
#include "../Lighting/LightManager.h"
#include "../Lighting/Light.h"
#include <string>
#include <cstring>

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
        
        uniform int numLights;
        uniform vec3 lightPositions[32];
        uniform vec3 lightColors[32];
        uniform float lightIntensities[32];
        uniform int lightTypes[32];
        uniform float lightRanges[32];
        uniform vec3 viewPos;

        uniform int hasShadow;
        uniform sampler2D shadowMap;
        uniform mat4 lightSpaceMatrix;
        uniform float shadowBias;
        
        float saturate(float x) { return clamp(x, 0.0, 1.0); }
        
        float DistributionGGX(vec3 N, vec3 H, float roughness) {
            float a      = roughness * roughness;
            float a2     = a * a;
            float NdotH  = max(dot(N, H), 0.0);
            float NdotH2 = NdotH * NdotH;
            float denom = (NdotH2 * (a2 - 1.0) + 1.0);
            return a2 / (3.14159265 * denom * denom);
        }
        
        float GeometrySchlickGGX(float NdotV, float roughness) {
            float r = (roughness + 1.0);
            float k = (r * r) / 8.0;
            float denom = NdotV * (1.0 - k) + k;
            return NdotV / denom;
        }
        
        float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
            float NdotV = max(dot(N, V), 0.0);
            float NdotL = max(dot(N, L), 0.0);
            float ggx1 = GeometrySchlickGGX(NdotV, roughness);
            float ggx2 = GeometrySchlickGGX(NdotL, roughness);
            return ggx1 * ggx2;
        }
        
        vec3 FresnelSchlick(float cosTheta, vec3 F0) {
            return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
        }

        float ComputeShadow(vec3 worldPos, vec3 N, vec3 L) {
            if (hasShadow == 0) return 0.0;
            vec4 lightSpacePos = lightSpaceMatrix * vec4(worldPos, 1.0);
            vec3 projCoords = lightSpacePos.xyz / max(lightSpacePos.w, 1e-5);
            projCoords = projCoords * 0.5 + 0.5;
            if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
                return 0.0;
            float currentDepth = projCoords.z;
            float closestDepth = texture(shadowMap, projCoords.xy).r;
            float bias = max(shadowBias * (1.0 - dot(N, L)), shadowBias * 0.2);
            return currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
        
        void main() {
            vec3 N = normalize(Normal);
            vec3 V = normalize(viewPos - FragPos);
            
            vec3 albedo = clamp(Color, 0.0, 1.0);
            float metallic = 0.0;   // can be driven from component later
            float roughness = 0.5;  // can be driven from component later
            float ao = 1.0;
            
            vec3 F0 = mix(vec3(0.04), albedo, metallic);
            
            vec3 Lo = vec3(0.0);
            for (int i = 0; i < numLights && i < 32; ++i) {
                vec3 L;
                float attenuation = 1.0;
                if (lightTypes[i] == 0) {
                    L = normalize(-lightPositions[i]);
                } else {
                    vec3 lightVec = lightPositions[i] - FragPos;
                    float distance = length(lightVec);
                    L = lightVec / max(distance, 1e-4);
                    if (distance > lightRanges[i]) continue;
                    attenuation = 1.0 / max(distance * distance, 1e-4);
                }
                
                vec3 H = normalize(V + L);
                float NDF = DistributionGGX(N, H, roughness);
                float G   = GeometrySmith(N, V, L, roughness);
                vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);
                
                vec3 numerator    = NDF * G * F;
                float denom       = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 1e-4;
                vec3 specular     = numerator / denom;
                
                vec3 kS = F;
                vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
                float NdotL = max(dot(N, L), 0.0);
                
                float shadow = 0.0;
                if (lightTypes[i] == 0 && hasShadow == 1) {
                    shadow = ComputeShadow(FragPos, N, L);
                }
                
                vec3 radiance = lightColors[i] * lightIntensities[i] * attenuation;
                Lo += (1.0 - shadow) * (kD * albedo / 3.14159265 + specular) * radiance * NdotL;
            }
            
            vec3 ambient = vec3(0.03) * albedo * ao;
            vec3 color = ambient + Lo;
            color = color / (color + vec3(1.0));
            color = pow(color, vec3(1.0/2.2));
            FragColor = vec4(color, 1.0);
        }
    )";
    
    m_forwardShader->LoadFromSource(vertexShaderSource, fragmentShaderSource);
    
    std::string transparentFragmentSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 FragPos;
        in vec3 Normal;
        in vec3 Color;
        
        uniform int numLights;
        uniform vec3 lightPositions[32];
        uniform vec3 lightColors[32];
        uniform float lightIntensities[32];
        uniform int lightTypes[32];
        uniform float lightRanges[32];
        uniform vec3 viewPos;
        uniform float alpha;
        
        float DistributionGGX(vec3 N, vec3 H, float roughness) {
            float a = roughness * roughness;
            float a2 = a * a;
            float NdotH = max(dot(N, H), 0.0);
            float d = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
            return a2 / (3.14159265 * d * d);
        }
        float GeometrySchlickGGX(float NdotV, float roughness) {
            float r = roughness + 1.0;
            float k = (r*r) / 8.0;
            return NdotV / (NdotV * (1.0 - k) + k);
        }
        float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
            float NdotV = max(dot(N, V), 0.0);
            float NdotL = max(dot(N, L), 0.0);
            float ggx1 = GeometrySchlickGGX(NdotV, roughness);
            float ggx2 = GeometrySchlickGGX(NdotL, roughness);
            return ggx1 * ggx2;
        }
        vec3 FresnelSchlick(float cosTheta, vec3 F0) {
            return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
        }
        
        void main() {
            vec3 N = normalize(Normal);
            vec3 V = normalize(viewPos - FragPos);
            vec3 albedo = clamp(Color, 0.0, 1.0);
            float metallic = 0.0;
            float roughness = 0.5;
            float ao = 1.0;
            vec3 F0 = mix(vec3(0.04), albedo, metallic);
            
            vec3 Lo = vec3(0.0);
            for (int i = 0; i < numLights && i < 32; ++i) {
                vec3 L;
                float attenuation = 1.0;
                if (lightTypes[i] == 0) {
                    L = normalize(-lightPositions[i]);
                } else {
                    vec3 lightVec = lightPositions[i] - FragPos;
                    float distance = length(lightVec);
                    if (distance > lightRanges[i]) continue;
                    L = lightVec / max(distance, 1e-4);
                    attenuation = 1.0 / max(distance*distance, 1e-4);
                }
                vec3 H = normalize(V + L);
                float NDF = DistributionGGX(N, H, roughness);
                float G   = GeometrySmith(N, V, L, roughness);
                vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);
                
                vec3 numerator = NDF * G * F;
                float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 1e-4;
                vec3 specular = numerator / denom;
                
                vec3 kS = F;
                vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
                float NdotL = max(dot(N, L), 0.0);
                
                vec3 radiance = lightColors[i] * lightIntensities[i] * attenuation;
                Lo += (kD * albedo / 3.14159265 + specular) * radiance * NdotL;
            }
            
            vec3 ambient = vec3(0.03) * albedo * ao;
            vec3 color = ambient + Lo;
            color = color / (color + vec3(1.0));
            color = pow(color, vec3(1.0/2.2));
            FragColor = vec4(color, alpha);
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

    RenderShadowPass(world);
    
    m_framebuffer->Bind();
    
    glViewport(0, 0, m_renderData.viewportWidth, m_renderData.viewportHeight);
    Logger::Debug("ForwardRenderPipeline: Set viewport to " + std::to_string(m_renderData.viewportWidth) + "x" + std::to_string(m_renderData.viewportHeight));
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);  // Disable backface culling to test if normals/winding are the issue
    
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
    
    CompositePass();
    
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
    static int frameCount = 0;
    std::string filename = "frames/frame" + std::to_string(frameCount) + ".png";
    
    Logger::Info("Saving frame " + std::to_string(frameCount) + " to " + filename);
    
    unsigned char* pixels = new unsigned char[m_renderData.viewportWidth * m_renderData.viewportHeight * 4];
    
    Logger::Debug("Reading pixels from bound framebuffer before unbinding");
    glReadPixels(0, 0, m_renderData.viewportWidth, m_renderData.viewportHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        Logger::Error("OpenGL error after glReadPixels: " + std::to_string(error));
    } else {
        Logger::Debug("glReadPixels completed successfully from framebuffer");
    }
    
    m_framebuffer->Unbind();
    
    unsigned char* flippedPixels = new unsigned char[m_renderData.viewportWidth * m_renderData.viewportHeight * 4];
    for (int y = 0; y < m_renderData.viewportHeight; y++) {
        std::memcpy(flippedPixels + (m_renderData.viewportHeight - 1 - y) * m_renderData.viewportWidth * 4, 
                   pixels + y * m_renderData.viewportWidth * 4, 
                   m_renderData.viewportWidth * 4);
    }
    
    int result = stbi_write_png(filename.c_str(), m_renderData.viewportWidth, m_renderData.viewportHeight, 4, flippedPixels, m_renderData.viewportWidth * 4);
    if (result) {
        Logger::Info("Successfully saved frame " + std::to_string(frameCount));
    } else {
        Logger::Error("Failed to save frame " + std::to_string(frameCount));
    }
    
    delete[] pixels;
    delete[] flippedPixels;
    
    frameCount++;
}

std::shared_ptr<Texture> ForwardRenderPipeline::GetFinalTexture() const {
    return m_colorTexture;
}

std::shared_ptr<FrameBuffer> ForwardRenderPipeline::GetFramebuffer() const {
    return m_framebuffer;
}

void ForwardRenderPipeline::Cleanup() {
    m_forwardShader.reset();
    m_transparentShader.reset();
    m_effectsShader.reset();
    m_compositeShader.reset();
    m_framebuffer.reset();
    m_colorTexture.reset();
    m_depthTexture.reset();
    
    m_initialized = false;
    Logger::Info("Forward rendering pipeline cleaned up");
}

void ForwardRenderPipeline::RenderOpaqueObjects(World* world) {
    if (!m_forwardShader || !world) {
        return;
    }
    
    m_forwardShader->Use();
    
    Logger::Debug("ForwardRenderPipeline: Setting view matrix");
    Logger::Debug("View matrix: [" + 
        std::to_string(m_renderData.viewMatrix.m[0]) + ", " + std::to_string(m_renderData.viewMatrix.m[1]) + ", " + std::to_string(m_renderData.viewMatrix.m[2]) + ", " + std::to_string(m_renderData.viewMatrix.m[3]) + "]");
    Logger::Debug("             [" + 
        std::to_string(m_renderData.viewMatrix.m[4]) + ", " + std::to_string(m_renderData.viewMatrix.m[5]) + ", " + std::to_string(m_renderData.viewMatrix.m[6]) + ", " + std::to_string(m_renderData.viewMatrix.m[7]) + "]");
    m_forwardShader->SetMatrix4("view", m_renderData.viewMatrix);
    Logger::Debug("ForwardRenderPipeline: Setting projection matrix");
    Logger::Debug("Projection matrix: [" + 
        std::to_string(m_renderData.projectionMatrix.m[0]) + ", " + std::to_string(m_renderData.projectionMatrix.m[1]) + ", " + std::to_string(m_renderData.projectionMatrix.m[2]) + ", " + std::to_string(m_renderData.projectionMatrix.m[3]) + "]");
    Logger::Debug("                   [" + 
        std::to_string(m_renderData.projectionMatrix.m[4]) + ", " + std::to_string(m_renderData.projectionMatrix.m[5]) + ", " + std::to_string(m_renderData.projectionMatrix.m[6]) + ", " + std::to_string(m_renderData.projectionMatrix.m[7]) + "]");
    m_forwardShader->SetMatrix4("projection", m_renderData.projectionMatrix);
    
    LightManager lightManager;
    lightManager.CollectLights(world);
    lightManager.ApplyBrightnessLimits();
    
    std::vector<LightManager::ShaderLightData> lightData;
    lightManager.GetShaderLightData(lightData);
    
    m_forwardShader->SetInt("numLights", static_cast<int>(lightData.size()));
    
    for (size_t i = 0; i < lightData.size() && i < MAX_LIGHTS; ++i) {
        std::string indexStr = std::to_string(i);
        m_forwardShader->SetVector3("lightPositions[" + indexStr + "]", lightData[i].position);
        m_forwardShader->SetVector3("lightColors[" + indexStr + "]", lightData[i].color);
        m_forwardShader->SetFloat("lightIntensities[" + indexStr + "]", lightData[i].intensity);
        m_forwardShader->SetInt("lightTypes[" + indexStr + "]", lightData[i].type);
        m_forwardShader->SetFloat("lightRanges[" + indexStr + "]", lightData[i].range);
    }
    
    Matrix4 invViewMatrix = m_renderData.viewMatrix.Inverted();
    Vector3 cameraPosition = Vector3(invViewMatrix.m[12], invViewMatrix.m[13], invViewMatrix.m[14]);
    m_forwardShader->SetVector3("viewPos", cameraPosition);

    int hasShadow = 0;
    Matrix4 lightSpace;
    int shadowTexUnit = 5;
    {
        LightManager lm2;
        lm2.CollectLights(world);
        Light* dirShadowLight = nullptr;
        for (auto* l : lm2.GetActiveLights()) {
            if (l && l->GetType() == LightType::Directional && l->GetCastShadows()) {
                dirShadowLight = l;
                break;
            }
        }
        if (dirShadowLight && dirShadowLight->GetShadowMap()) {
            dirShadowLight->InitializeShadowMap();
            auto sm = dirShadowLight->GetShadowMap();
            auto fb = dirShadowLight->GetShadowFramebuffer();
            if (sm && fb) {
                hasShadow = 1;
                lightSpace = dirShadowLight->GetLightSpaceMatrix();
                sm->Bind(shadowTexUnit);
                m_forwardShader->SetInt("shadowMap", shadowTexUnit);
                m_forwardShader->SetMatrix4("lightSpaceMatrix", lightSpace);
                m_forwardShader->SetFloat("shadowBias", dirShadowLight->GetShadowBias());
            }
        }
    }
    m_forwardShader->SetInt("hasShadow", hasShadow);
    
    int entitiesRendered = 0;
    
    for (const auto& entity : world->GetEntities()) {
        if (world->HasComponent<TransformComponent>(entity)) {
            auto* transformComp = world->GetComponent<TransformComponent>(entity);
            if (transformComp) {
                Matrix4 modelMatrix = transformComp->transform.GetLocalToWorldMatrix();
                Vector3 position = transformComp->transform.GetPosition();
                Logger::Debug("Entity position: (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + ")");
                m_forwardShader->SetMatrix4("model", modelMatrix);
                
                static Mesh cubeMesh = Mesh::CreateCube(1.0f);
                static bool meshUploaded = false;
                if (!meshUploaded) {
                    Logger::Debug("ForwardRenderPipeline: Attempting to upload cube mesh...");
                    cubeMesh.Upload();
                    meshUploaded = true;
                    Logger::Debug("ForwardRenderPipeline: Cube mesh upload completed, meshUploaded = true");
                }
                
                if (!cubeMesh.GetVertices().empty()) {
                    cubeMesh.Draw();
                    entitiesRendered++;
                }
            }
        }
    }
    
    Logger::Debug("Forward rendering: Rendered " + std::to_string(entitiesRendered) + " entities");
}

void ForwardRenderPipeline::RenderTransparentObjects(World* world) {
    if (!m_transparentShader) {
        return;
    }
    
    m_transparentShader->Use();
    
    LightManager transparentLightManager;
    transparentLightManager.CollectLights(world);
    transparentLightManager.ApplyBrightnessLimits();
    
    std::vector<LightManager::ShaderLightData> transparentLightData;
    transparentLightManager.GetShaderLightData(transparentLightData);
    
    m_transparentShader->SetInt("numLights", static_cast<int>(transparentLightData.size()));
    
    for (size_t i = 0; i < transparentLightData.size() && i < MAX_LIGHTS; ++i) {
        std::string indexStr = std::to_string(i);
        m_transparentShader->SetVector3("lightPositions[" + indexStr + "]", transparentLightData[i].position);
        m_transparentShader->SetVector3("lightColors[" + indexStr + "]", transparentLightData[i].color);
        m_transparentShader->SetFloat("lightIntensities[" + indexStr + "]", transparentLightData[i].intensity);
        m_transparentShader->SetInt("lightTypes[" + indexStr + "]", transparentLightData[i].type);
        m_transparentShader->SetFloat("lightRanges[" + indexStr + "]", transparentLightData[i].range);
    }
    
    Matrix4 invViewMatrix = m_renderData.viewMatrix.Inverted();
    Vector3 cameraPosition = Vector3(invViewMatrix.m[12], invViewMatrix.m[13], invViewMatrix.m[14]);
    m_transparentShader->SetVector3("viewPos", cameraPosition);
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

void ForwardRenderPipeline::CompositePass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_renderData.viewportWidth, m_renderData.viewportHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glDisable(GL_DEPTH_TEST);
    
    if (!m_compositeShader) {
        m_compositeShader = std::make_shared<Shader>();
        std::string vertexSource = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aTexCoord;
            
            out vec2 TexCoord;
            
            void main() {
                TexCoord = aTexCoord;
                gl_Position = vec4(aPos, 0.0, 1.0);
            }
        )";
        
        std::string fragmentSource = R"(
            #version 330 core
            out vec4 FragColor;
            
            in vec2 TexCoord;
            uniform sampler2D finalTexture;
            
            void main() {
                FragColor = texture(finalTexture, TexCoord);
            }
        )";
        
        m_compositeShader->LoadFromSource(vertexSource, fragmentSource);
    }
    
    m_compositeShader->Use();
    
    auto finalTexture = m_framebuffer->GetColorTexture(0);
    if (finalTexture) {
        finalTexture->Bind(0);
        m_compositeShader->SetInt("finalTexture", 0);
    }
    
    RenderFullscreenQuad();
    
    glEnable(GL_DEPTH_TEST);
}

void ForwardRenderPipeline::RenderShadowPass(World* world) {
    if (!world) return;

    LightManager lm;
    lm.CollectLights(world);

    Light* dirShadowLight = nullptr;
    for (auto* l : lm.GetActiveLights()) {
        if (l && l->GetType() == LightType::Directional && l->GetCastShadows()) {
            dirShadowLight = l;
            break;
        }
    }
    if (!dirShadowLight) return;

    dirShadowLight->InitializeShadowMap();
    auto fb = dirShadowLight->GetShadowFramebuffer();
    auto sm = dirShadowLight->GetShadowMap();
    if (!fb || !sm) return;

    int size = dirShadowLight->GetShadowMapSize();
    fb->Bind();
    glViewport(0, 0, size, size);
    glClear(GL_DEPTH_BUFFER_BIT);

    if (!m_depthShader) {
        m_depthShader = std::make_shared<Shader>();
        std::string vsrc = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            uniform mat4 model;
            uniform mat4 lightSpaceMatrix;
            void main() {
                gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
            }
        )";
        std::string fsrc = R"(
            #version 330 core
            void main() { }
        )";
        m_depthShader->LoadFromSource(vsrc, fsrc);
    }

    m_depthShader->Use();
    Matrix4 lightSpace = dirShadowLight->GetLightSpaceMatrix();
    m_depthShader->SetMatrix4("lightSpaceMatrix", lightSpace);

    static Mesh cubeMesh = Mesh::CreateCube(1.0f);
    static bool uploaded = false;
    if (!uploaded) { cubeMesh.Upload(); uploaded = true; }

    for (const auto& entity : world->GetEntities()) {
        if (world->HasComponent<TransformComponent>(entity)) {
            auto* tc = world->GetComponent<TransformComponent>(entity);
            if (!tc) continue;
            Matrix4 model = tc->transform.GetLocalToWorldMatrix();
            m_depthShader->SetMatrix4("model", model);
            if (!cubeMesh.GetVertices().empty()) {
                cubeMesh.Draw();
            }
        }
    }

    fb->Unbind();
    glViewport(0, 0, m_renderData.viewportWidth, m_renderData.viewportHeight);
}

void ForwardRenderPipeline::RenderFullscreenQuad() {
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
    }
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

}
