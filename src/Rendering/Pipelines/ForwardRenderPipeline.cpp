#include "ForwardRenderPipeline.h"
#include "../../Core/Logging/Logger.h"
#include "../../Core/Profiling/Profiler.h"
#include "../../Core/ECS/World.h"
#include "../../Core/Components/TransformComponent.h"
#include "../../Core/Components/MeshComponent.h"
#include "../Meshes/Mesh.h"
#include "../Core/OpenGLHeaders.h"
#include "../Core/stb_image_write.h"
#include "../Lighting/LightManager.h"
#include "../Lighting/Light.h"
#include "../Lighting/LightOcclusion.h"
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
        #version 430 core
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
        #version 430 core
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

        uniform int lightHasShadow[32];
        uniform int shadowType[32];                 // 0 = Directional, 1 = Point, 2 = Spot
        uniform sampler2D shadowMaps2D[8];
        uniform samplerCube shadowMapsCube[8];
        uniform int shadowSamplerIndex[32];         // index into shadowMaps2D or shadowMapsCube
        uniform mat4 lightSpaceMatrices[32];        // for dir/spot
        uniform float shadowBiases[32];
        uniform float shadowTexelSizes[32];         // 1.0 / mapSize for dir/spot
        uniform vec3 shadowLightPositions[32];      // for point
        uniform float shadowNearPlanes[32];         // for point
        uniform float shadowFarPlanes[32];          // for point

        struct VolumeHeader { int lightIndex; int vertCount; int baseOffset; int farOffset; };
        layout(std430, binding = 3) buffer ShadowVolumeHeaders { VolumeHeader headers[]; };
        layout(std430, binding = 4) buffer ShadowVolumeVertices { vec4 vertices[]; };
        uniform int numVolumeHeaders;

        bool insidePrism(int vertCount, int baseOffset, int farOffset, vec3 P) {
            if (vertCount < 3) return false;
            for (int i = 0; i < vertCount; ++i) {
                vec3 a0 = vertices[baseOffset + i].xyz;
                vec3 a1 = vertices[baseOffset + ((i + 1) % vertCount)].xyz;
                vec3 b0 = vertices[farOffset + i].xyz;
                vec3 edge = a1 - a0;
                vec3 extrude = b0 - a0;
                vec3 n = normalize(cross(edge, extrude));
                if (dot(n, P - a0) > 0.0) return false;
            }
            return true;
        }

        bool insideAnyLightVolume(int lightIdx, vec3 P) {
            for (int h = 0; h < numVolumeHeaders; ++h) {
                if (headers[h].lightIndex != lightIdx) continue;
                if (insidePrism(headers[h].vertCount, headers[h].baseOffset, headers[h].farOffset, P)) return true;
            }
            return false;
        }
        
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

        float ComputeShadowDir(int li, vec3 worldPos, vec3 N, vec3 L) {
            vec4 lsp = lightSpaceMatrices[li] * vec4(worldPos, 1.0);
            vec3 proj = lsp.xyz / max(lsp.w, 1e-5);
            proj = proj * 0.5 + 0.5;
            if (proj.z > 1.0 || proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0) return 0.0;
            float currentDepth = proj.z;
            float bias = max(shadowBiases[li] * (1.0 - dot(N, L)), shadowBiases[li] * 0.2);
            float shadow = 0.0;
            int idx = shadowSamplerIndex[li];
            for (int x = -1; x <= 1; ++x) {
                for (int y = -1; y <= 1; ++y) {
                    vec2 offset = vec2(x, y) * shadowTexelSizes[li];
                    float closestDepth = texture(shadowMaps2D[idx], proj.xy + offset).r;
                    shadow += (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
                }
            }
            return shadow / 9.0;
        }
        float LinearizeDepth(float depth, float nearP, float farP) {
            float z = depth * 2.0 - 1.0;
            return (2.0 * nearP * farP) / (farP + nearP - z * (farP - nearP));
        }
        float ComputeShadowPoint(int li, vec3 worldPos) {
            vec3 Lvec = worldPos - shadowLightPositions[li];
            float dist = length(Lvec);
            int idx = shadowSamplerIndex[li];
            float bias = shadowBiases[li];
            float shadow = 0.0;
            int samples = 4;
            vec3 dir = normalize(Lvec);
            vec3 offsets[4] = vec3[](
                vec3( 1,  1,  1),
                vec3(-1,  1, -1),
                vec3( 1, -1, -1),
                vec3(-1, -1,  1)
            );
            for (int i = 0; i < samples; ++i) {
                vec3 probe = dir + offsets[i] * 0.01;
                float depthSample = texture(shadowMapsCube[idx], probe).r;
                float sampleDist = LinearizeDepth(depthSample, shadowNearPlanes[li], shadowFarPlanes[li]);
                shadow += (dist - bias > sampleDist) ? 1.0 : 0.0;
            }
            return shadow / float(samples);
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
                if (lightHasShadow[i] == 1) {
                    if (shadowType[i] == 1) {
                        shadow = ComputeShadowPoint(i, FragPos);
                    } else {
                        shadow = ComputeShadowDir(i, FragPos, N, L);
                    }
                }
                if (insideAnyLightVolume(i, FragPos)) {
                    shadow = 1.0;
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
    if (!m_depthShader) {
        std::string depthVS = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            uniform mat4 model;
            uniform mat4 lightSpaceMatrix;
            void main() {
                gl_Position = lightSpaceMatrix * vec4(aPos, 1.0);
            }
        )";
        std::string depthFS = R"(
            #version 330 core
            void main() { }
        )";
        m_depthShader = std::make_shared<Shader>();
        m_depthShader->LoadFromSource(depthVS, depthFS);
    }
    m_initialized = true;
    Logger::Info("Forward rendering pipeline initialized successfully");
    return true;
}

void ForwardRenderPipeline::Render(World* world) {
    PROFILE_GPU("ForwardRenderPipeline::Render");
    if (!m_initialized || !world) {
        return;
    }

    {
        PROFILE_GPU("ForwardPipeline::ShadowPass");
        RenderShadowPass(world);
    }
    
    m_framebuffer->Bind();
    
    glViewport(0, 0, m_renderData.viewportWidth, m_renderData.viewportHeight);
    Logger::Debug("ForwardRenderPipeline: Set viewport to " + std::to_string(m_renderData.viewportWidth) + "x" + std::to_string(m_renderData.viewportHeight));

    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);  // Disable backface culling to test if normals/winding are the issue
    
    {
        PROFILE_GPU("ForwardPipeline::SetupLighting");
        SetupLighting();
    }
    
    {
        PROFILE_GPU("ForwardPipeline::RenderOpaqueObjects");
        RenderOpaqueObjects(world);
    }
    
    if (m_transparencyEnabled) {
        PROFILE_GPU("ForwardPipeline::TransparentObjects");
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        SortTransparentObjects(world);
        RenderTransparentObjects(world);
        
        glDisable(GL_BLEND);
    }
    
    {
        PROFILE_GPU("ForwardPipeline::SpecialEffects");
        RenderSpecialEffects(world);
    }
    
    {
        PROFILE_GPU("ForwardPipeline::CompositePass");
        CompositePass();
    }
    
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
    m_framebuffer->Unbind();
}

std::shared_ptr<Texture> ForwardRenderPipeline::GetFinalTexture() const {
    return m_framebuffer ? m_framebuffer->GetColorTexture(0) : nullptr;
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
    m_forwardShader->SetMatrix4("projection", m_renderData.projectionMatrix);
    
    LightManager lmEmbed;
    lmEmbed.CollectLights(world);
    auto actEmbed = lmEmbed.GetActiveLights();

    int lightHasShadowArr[32] = {0};
    int shadowTypeArr[32] = {0};
    int shadowSamplerIdxArr[32] = {0};
    float shadowBiasesArr[32] = {0};
    float shadowTexelSizesArr[32] = {0};
    Vector3 shadowLightPosArr[32];
    float shadowNearArr[32] = {0};
    float shadowFarArr[32] = {0};
    Matrix4 lightSpaceMatricesArr[32];

    int used2D = 0;
    int usedCube = 0;
    const int base2D = 5;
    const int baseCube = base2D + 8;

    for (int i = 0; i < 8; ++i) {
        m_forwardShader->SetInt("shadowMaps2D[" + std::to_string(i) + "]", base2D + i);
        m_forwardShader->SetInt("shadowMapsCube[" + std::to_string(i) + "]", baseCube + i);
    }

    for (size_t i = 0; i < actEmbed.size() && i < 32; ++i) {
        Light* l = actEmbed[i];
        if (!l || !l->GetCastShadows()) continue;
        l->InitializeShadowMap();
        auto sm = l->GetShadowMap();
        auto fb = l->GetShadowFramebuffer();
        if (!sm || !fb) continue;

        lightHasShadowArr[i] = 1;
        shadowBiasesArr[i] = l->GetShadowBias();

        if (l->GetType() == LightType::Point) {
            shadowTypeArr[i] = 1;
            shadowLightPosArr[i] = l->GetPosition();
            shadowNearArr[i] = l->GetData().shadowNearPlane;
            shadowFarArr[i] = l->GetData().shadowFarPlane;

            int cubeIdx = usedCube;
            shadowSamplerIdxArr[i] = cubeIdx;
            int unit = baseCube + cubeIdx;
            sm->Bind(unit);
            usedCube++;
        } else {
            shadowTypeArr[i] = (l->GetType() == LightType::Directional) ? 0 : 2;
            lightSpaceMatricesArr[i] = l->GetLightSpaceMatrix();
            shadowTexelSizesArr[i] = 1.0f / static_cast<float>(l->GetShadowMapSize());

            int idx2d = used2D;
            shadowSamplerIdxArr[i] = idx2d;
            int unit = base2D + idx2d;
            sm->Bind(unit);
            used2D++;
        }
    }

    for (int i = 0; i < 32; ++i) {
        m_forwardShader->SetInt("lightHasShadow[" + std::to_string(i) + "]", lightHasShadowArr[i]);
        m_forwardShader->SetInt("shadowType[" + std::to_string(i) + "]", shadowTypeArr[i]);
        m_forwardShader->SetInt("shadowSamplerIndex[" + std::to_string(i) + "]", shadowSamplerIdxArr[i]);
        m_forwardShader->SetFloat("shadowBiases[" + std::to_string(i) + "]", shadowBiasesArr[i]);
        m_forwardShader->SetFloat("shadowTexelSizes[" + std::to_string(i) + "]", shadowTexelSizesArr[i]);
        m_forwardShader->SetVector3("shadowLightPositions[" + std::to_string(i) + "]", shadowLightPosArr[i]);
        m_forwardShader->SetFloat("shadowNearPlanes[" + std::to_string(i) + "]", shadowNearArr[i]);
        m_forwardShader->SetFloat("shadowFarPlanes[" + std::to_string(i) + "]", shadowFarArr[i]);
        m_forwardShader->SetMatrix4("lightSpaceMatrices[" + std::to_string(i) + "]", lightSpaceMatricesArr[i]);
    }
    
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

    if (!m_lightOcclusion) {
        m_lightOcclusion = std::make_unique<LightOcclusion>();
        if (world->GetPhysicsWorld()) {
            m_lightOcclusion->Initialize(world->GetPhysicsWorld());
        } else {
            Logger::Error("LightOcclusion cannot initialize: PhysicsWorld is null");
        }
    }
    std::vector<unsigned int> headersCPU; // int-packed: lightIndex, vertCount, baseOffset, farOffset
    std::vector<float> vertsCPU;          // vec4 packed as 4 floats per vertex

    int totalHeaders = 0;
    int baseOffset = 0;
    int farOffset = 0;

    for (size_t li = 0; li < actEmbed.size(); ++li) {
        Light* l = actEmbed[li];
        if (!l) continue;
        float dirFar = 1000.0f;
        m_lightOcclusion->BuildShadowVolumesForLight(l, world, static_cast<int>(li), dirFar);
        const auto* vols = m_lightOcclusion->GetVolumesForLight(l);
        if (!vols) continue;

        for (const auto& v : *vols) {
            int vc = static_cast<int>(v.basePolygon.size());
            if (vc < 3) continue;

            int thisBaseOffset = baseOffset;
            int thisFarOffset = farOffset;

            for (int k = 0; k < vc; ++k) {
                const Vector3& bp = v.basePolygon[k];
                vertsCPU.push_back(bp.x); vertsCPU.push_back(bp.y); vertsCPU.push_back(bp.z); vertsCPU.push_back(0.0f);
            }
            baseOffset += vc;

            for (int k = 0; k < vc; ++k) {
                const Vector3& fp = v.farPolygon[k];
                vertsCPU.push_back(fp.x); vertsCPU.push_back(fp.y); vertsCPU.push_back(fp.z); vertsCPU.push_back(0.0f);
            }
            farOffset += vc;

            headersCPU.push_back(static_cast<unsigned int>(v.lightIndex));
            headersCPU.push_back(static_cast<unsigned int>(vc));
            headersCPU.push_back(static_cast<unsigned int>(thisBaseOffset));
            headersCPU.push_back(static_cast<unsigned int>(thisFarOffset));
            totalHeaders++;
        }
    }

    if (m_shadowVolumeHeadersSSBO == 0) {
        glGenBuffers(1, &m_shadowVolumeHeadersSSBO);
    }
    if (m_shadowVolumeVerticesSSBO == 0) {
        glGenBuffers(1, &m_shadowVolumeVerticesSSBO);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_shadowVolumeHeadersSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, headersCPU.size() * sizeof(unsigned int), headersCPU.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_shadowVolumeHeadersSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_shadowVolumeVerticesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertsCPU.size() * sizeof(float), vertsCPU.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_shadowVolumeVerticesSSBO);

    Logger::Debug("Shadow volumes: headers=" + std::to_string(totalHeaders) + ", headerInts=" + std::to_string(headersCPU.size()) + ", vertsFloats=" + std::to_string(vertsCPU.size()));
    m_forwardShader->SetInt("numVolumeHeaders", totalHeaders);

    Matrix4 invViewMatrix = m_renderData.viewMatrix.Inverted();
    Vector3 cameraPosition = Vector3(invViewMatrix.m[12], invViewMatrix.m[13], invViewMatrix.m[14]);
    m_forwardShader->SetVector3("viewPos", cameraPosition);

    GLint currProg = 0, vao=0, ebo=0, abo=0, dfb=0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProg);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ebo);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &abo);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &dfb);
    Logger::Debug("Forward state pre-draw: prog=" + std::to_string(currProg) +
                  " vao=" + std::to_string(vao) +
                  " ebo=" + std::to_string(ebo) +
                  " abo=" + std::to_string(abo) +
                  " dfb=" + std::to_string(dfb));
    if (currProg != 0) {
        glValidateProgram(static_cast<GLuint>(currProg));
        GLint ok = GL_FALSE;
        glGetProgramiv(static_cast<GLuint>(currProg), GL_VALIDATE_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetProgramiv(static_cast<GLuint>(currProg), GL_INFO_LOG_LENGTH, &len);
            std::string vlog;
            if (len > 1) {
                vlog.resize(static_cast<size_t>(len));
                glGetProgramInfoLog(static_cast<GLuint>(currProg), len, nullptr, vlog.data());
            }
            Logger::Error(std::string("Forward program validation failed: ") + (vlog.empty() ? "(no log)" : vlog.c_str()));
        } else {
            Logger::Debug("Forward program validation OK");
        }
    } else {
        Logger::Error("Forward draw with no current program bound");
    }
    
    int entitiesRendered = 0;
    
    for (const auto& entity : world->GetEntities()) {
        if (world->HasComponent<TransformComponent>(entity) && world->HasComponent<MeshComponent>(entity)) {
            auto* transformComp = world->GetComponent<TransformComponent>(entity);
            auto* meshComp = world->GetComponent<MeshComponent>(entity);
            if (transformComp && meshComp && meshComp->HasMesh() && meshComp->IsVisible()) {
                Matrix4 modelMatrix = transformComp->transform.GetLocalToWorldMatrix();
                Vector3 position = transformComp->transform.GetPosition();
                Logger::Debug("Entity position: (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + ")");
                m_forwardShader->SetMatrix4("model", modelMatrix);
                
                meshComp->GetMesh()->Draw();
                entitiesRendered++;
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

    if (!m_cachedLightManager) {
        m_cachedLightManager = std::make_unique<LightManager>();
    }
    m_cachedLightManager->CollectLights(world);
    auto act = m_cachedLightManager->GetActiveLights();
    if (act.empty()) return;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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

    for (auto* shadowLight : act) {
        if (!shadowLight || !shadowLight->GetCastShadows()) continue;
        shadowLight->InitializeShadowMap();
        auto fb = shadowLight->GetShadowFramebuffer();
        auto sm = shadowLight->GetShadowMap();
        if (!fb || !sm) continue;

        int sz = shadowLight->GetData().shadowMapSize;

        if (shadowLight->GetType() == LightType::Point) {
            static const Vector3 dirs[6] = {
                Vector3(1,0,0), Vector3(-1,0,0), Vector3(0,1,0),
                Vector3(0,-1,0), Vector3(0,0,1), Vector3(0,0,-1)
            };
            static const Vector3 ups[6] = {
                Vector3(0,-1,0), Vector3(0,-1,0), Vector3(0,0,1),
                Vector3(0,0,-1), Vector3(0,-1,0), Vector3(0,-1,0)
            };
            Matrix4 proj = shadowLight->GetProjectionMatrix();

            Vector3 lp = shadowLight->GetPosition();
            for (int face = 0; face < 6; ++face) {
                size_t shadowDrawnThisFace = 0;
                fb->Bind();
                fb->AttachDepthCubeFace(sm, face);
                glViewport(0, 0, sz, sz);
                glClear(GL_DEPTH_BUFFER_BIT);

                Matrix4 view = Matrix4::LookAt(lp, lp + dirs[face], ups[face]);
                Matrix4 lightSpace = proj * view;
                m_depthShader->SetMatrix4("lightSpaceMatrix", lightSpace);

                Vector3 cameraPos = Vector3(m_renderData.viewMatrix.Inverted().m[12], 
                                           m_renderData.viewMatrix.Inverted().m[13], 
                                           m_renderData.viewMatrix.Inverted().m[14]);
                
                for (const auto& e : world->GetEntities()) {
                    auto* t = world->GetComponent<TransformComponent>(e);
                    auto* mc = world->GetComponent<MeshComponent>(e);
                    if (!t || !mc) continue;
                    auto mesh = mc->GetMesh();
                    if (!mesh) continue;

                    float distance = (t->transform.GetPosition() - cameraPos).Length();
                    if (distance > 100.0f) continue;

                    Matrix4 model = t->transform.GetLocalToWorldMatrix();
                    m_depthShader->SetMatrix4("model", model);
                    ++shadowDrawnThisFace;

                    mesh->Draw();
                }
                Logger::Debug(std::string("Shadow pass (point) face ") + std::to_string(face) + " drew " + std::to_string(shadowDrawnThisFace) + " meshes");
            }
            fb->Unbind();
        } else {
            fb->Bind();
            glViewport(0, 0, sz, sz);
            glClear(GL_DEPTH_BUFFER_BIT);
            size_t shadowDrawn = 0;

            Matrix4 lightSpace = shadowLight->GetLightSpaceMatrix();
            m_depthShader->SetMatrix4("lightSpaceMatrix", lightSpace);

            Vector3 cameraPos = Vector3(m_renderData.viewMatrix.Inverted().m[12], 
                                       m_renderData.viewMatrix.Inverted().m[13], 
                                       m_renderData.viewMatrix.Inverted().m[14]);
            
            for (const auto& e : world->GetEntities()) {
                auto* t = world->GetComponent<TransformComponent>(e);
                auto* mc = world->GetComponent<MeshComponent>(e);
                if (!t || !mc) continue;
                auto mesh = mc->GetMesh();
                if (!mesh) continue;

                float distance = (t->transform.GetPosition() - cameraPos).Length();
                if (distance > 100.0f) continue;

                Matrix4 model = t->transform.GetLocalToWorldMatrix();
                m_depthShader->SetMatrix4("model", model);
                ++shadowDrawn;

                mesh->Draw();
            }
            Logger::Debug("Shadow pass drew " + std::to_string(shadowDrawn) + " meshes");
            fb->Unbind();
        }
    }

    glCullFace(GL_BACK);
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
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
