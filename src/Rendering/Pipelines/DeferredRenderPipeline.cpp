#include "DeferredRenderPipeline.h"
#include "../../Core/Profiling/Profiler.h"
#include "../../Core/ECS/World.h"
#include "../../Core/Components/TransformComponent.h"
#include "../../Core/Components/MeshComponent.h"
#include "../../Core/Logging/Logger.h"
#include "../Meshes/Mesh.h"
#include "../Core/OpenGLHeaders.h"
#include "../Lighting/LightManager.h"
#include "../Lighting/Light.h"
#include "../Lighting/LightOcclusion.h"
#include "../Core/FrameCapture.h"

#include <string>
#include <cstring>

namespace GameEngine {

const int DeferredRenderPipeline::SHADOW_MAP_SIZE;

DeferredRenderPipeline::DeferredRenderPipeline() = default;
DeferredRenderPipeline::~DeferredRenderPipeline() = default;

bool DeferredRenderPipeline::Initialize(int width, int height) {
    m_width = width;
    m_height = height;
    
    Logger::Info("Initializing Deferred Rendering Pipeline...");
    
    CreateGBuffer();
    CreateShaders();

    const int TILE_SIZE = 16;
    m_tilesX = (m_width + TILE_SIZE - 1) / TILE_SIZE;
    m_tilesY = (m_height + TILE_SIZE - 1) / TILE_SIZE;

    if (m_lightGridSSBO == 0) {
        glGenBuffers(1, &m_lightGridSSBO);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightGridSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(m_tilesX * m_tilesY * sizeof(int) * 2), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightGridSSBO);

    if (m_lightIndexSSBO == 0) {
        glGenBuffers(1, &m_lightIndexSSBO);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightIndexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(1024 * 1024 * sizeof(int)), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_lightIndexSSBO);

    if (!m_tiledCullShader) {
        m_tiledCullShader = std::make_shared<Shader>();
        m_tiledCullShader->LoadComputeShader("src/Rendering/Shaders/tiled_light_cull.comp");
    }
    
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
    if (m_lightGridSSBO) {
        glDeleteBuffers(1, &m_lightGridSSBO);
        m_lightGridSSBO = 0;
    }
    if (m_lightIndexSSBO) {
        glDeleteBuffers(1, &m_lightIndexSSBO);
        m_lightIndexSSBO = 0;
    }
    m_tiledCullShader.reset();
}

void DeferredRenderPipeline::BeginFrame(const RenderData& renderData) {
    m_renderData = renderData;
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    m_gBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DeferredRenderPipeline::Render(World* world) {
    PROFILE_GPU("DeferredRenderPipeline::Render");
    {
        PROFILE_GPU("DeferredPipeline::ShadowPass");
        ShadowPass(world);
    }
    {
        PROFILE_GPU("DeferredPipeline::GeometryPass");
        GeometryPass(world);
    }
    {
        PROFILE_GPU("DeferredPipeline::LightingPass");
        LightingPass(world);
    }
    {
        PROFILE_GPU("DeferredPipeline::CompositePass");
        CompositePass();
    }
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

    const int TILE_SIZE = 16;
    m_tilesX = (m_width + TILE_SIZE - 1) / TILE_SIZE;
    m_tilesY = (m_height + TILE_SIZE - 1) / TILE_SIZE;
    if (m_lightGridSSBO) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lightGridSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(m_tilesX * m_tilesY * sizeof(int) * 2), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightGridSSBO);
    }
}

std::shared_ptr<Texture> DeferredRenderPipeline::GetFinalTexture() const {
    return m_lightingBuffer ? m_lightingBuffer->GetColorTexture(0) : nullptr;
}

std::shared_ptr<FrameBuffer> DeferredRenderPipeline::GetFramebuffer() const {
    return std::shared_ptr<FrameBuffer>(m_lightingBuffer.get(), [](FrameBuffer*){});
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
    
    m_shadowMapBuffer = std::make_unique<FrameBuffer>(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    m_shadowMapBuffer->AddDepthAttachment(TextureFormat::Depth24);
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
        uniform sampler2D shadowMap;
        
        uniform int numLights;
        uniform vec3 lightPositions[32];
        uniform vec3 lightColors[32];
        uniform float lightIntensities[32];
        uniform int lightTypes[32];
        uniform float lightRanges[32];
        uniform vec3 viewPos;
        uniform mat4 lightSpaceMatrix;
        uniform ivec2 screenSize;

        layout(std430, binding = 0) buffer LightGrid { ivec2 grid[]; };
        layout(std430, binding = 1) buffer LightIndex { int indices[]; };

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
        
        float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
            vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
            projCoords = projCoords * 0.5 + 0.5;
            
            if(projCoords.z > 1.0) return 0.0;
            
            float closestDepth = texture(shadowMap, projCoords.xy).r;
            float currentDepth = projCoords.z;
            
            float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
            float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
            
            return shadow;
        }

        ivec2 tileOf(vec2 fragCoord) {
            ivec2 tile = ivec2(fragCoord / 16.0);
            int tilesX = (screenSize.x + 15) / 16;
            int tilesY = (screenSize.y + 15) / 16;
            tile.x = clamp(tile.x, 0, max(tilesX - 1, 0));
            tile.y = clamp(tile.y, 0, max(tilesY - 1, 0));
            return tile;
        }
        
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

            ivec2 tiles = ivec2((screenSize.x + 15) / 16, (screenSize.y + 15) / 16);
            ivec2 tile = tileOf(gl_FragCoord.xy);
            int tileIndex = tile.y * max(tiles.x, 1) + tile.x;
            ivec2 oc = grid[tileIndex];
            int off = oc.x;
            int cnt = oc.y;
            
            for(int k = 0; k < cnt; ++k) {
                int i = indices[off + k];
                if (i < 0 || i >= numLights) continue;

                vec3 lightContribution = vec3(0.0);
                if (insideAnyLightVolume(i, fragPos)) { continue; }
                
                if(lightTypes[i] == 0) {
                    vec3 lightDir = normalize(-lightPositions[i]);
                    
                    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
                    float shadow = ShadowCalculation(fragPosLightSpace, normal, lightDir);
                    
                    float diff = max(dot(normal, lightDir), 0.0);
                    vec3 diffuse = diff * lightColors[i] * lightIntensities[i] * albedo;
                    
                    vec3 reflectDir = reflect(-lightDir, normal);
                    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
                    vec3 specular = spec * lightColors[i] * lightIntensities[i];
                    
                    lightContribution = (diffuse + specular) * (1.0 - shadow);
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

void DeferredRenderPipeline::ShadowPass(World* world) {
    if (!m_shadowMapBuffer) return;
    
    m_lightSpaceMatrix = Matrix4::Identity();
    
    if (!m_cachedLightManager) {
        m_cachedLightManager = std::make_unique<LightManager>();
    }
    m_cachedLightManager->CollectLights(world);
    
    auto lights = m_cachedLightManager->GetActiveLights();
    
    m_shadowMapBuffer->Bind();
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    for (auto* light : lights) {
        if (!light || !light->GetCastShadows()) continue;
        
        light->InitializeShadowMap();
        m_lightSpaceMatrix = light->GetLightSpaceMatrix();
        
        if (m_geometryShader) {
            m_geometryShader->Use();
            m_geometryShader->SetMatrix4("uView", light->GetViewMatrix());
            m_geometryShader->SetMatrix4("uProjection", light->GetProjectionMatrix());
            
            Vector3 cameraPos = Vector3(m_renderData.viewMatrix.Inverted().m[12], 
                                       m_renderData.viewMatrix.Inverted().m[13], 
                                       m_renderData.viewMatrix.Inverted().m[14]);
            
            for (const auto& entity : world->GetEntities()) {
                if (world->HasComponent<TransformComponent>(entity) && world->HasComponent<MeshComponent>(entity)) {
                    auto* transformComp = world->GetComponent<TransformComponent>(entity);
                    auto* meshComp = world->GetComponent<MeshComponent>(entity);
                    if (transformComp && meshComp && meshComp->HasMesh() && meshComp->IsVisible()) {
                        float distance = (transformComp->transform.GetPosition() - cameraPos).Length();
                        if (distance > 100.0f) continue;
                        
                        Matrix4 modelMatrix = transformComp->transform.GetLocalToWorldMatrix();
                        m_geometryShader->SetMatrix4("uModel", modelMatrix);
                        meshComp->GetMesh()->Draw();
                    }
                }
            }
        }
        
        break;
    }
    
    m_shadowMapBuffer->Unbind();
}

void DeferredRenderPipeline::GeometryPass(World* world) {
    m_gBuffer->Bind();
    
    if (m_geometryShader) {
        m_geometryShader->Use();
        m_geometryShader->SetMatrix4("uView", m_renderData.viewMatrix);
        m_geometryShader->SetMatrix4("uProjection", m_renderData.projectionMatrix);
    }
    
    if (world) {
        Vector3 cameraPos = Vector3(m_renderData.viewMatrix.Inverted().m[12], 
                                   m_renderData.viewMatrix.Inverted().m[13], 
                                   m_renderData.viewMatrix.Inverted().m[14]);
        
        int entityCount = 0;
        for (const auto& entity : world->GetEntities()) {
            if (world->HasComponent<TransformComponent>(entity) && world->HasComponent<MeshComponent>(entity)) {
                auto* transformComp = world->GetComponent<TransformComponent>(entity);
                auto* meshComp = world->GetComponent<MeshComponent>(entity);
                if (transformComp && meshComp && meshComp->HasMesh() && meshComp->IsVisible()) {
                    float distance = (transformComp->transform.GetPosition() - cameraPos).Length();
                    if (distance > 100.0f) continue;
                    
                    Matrix4 modelMatrix = transformComp->transform.GetLocalToWorldMatrix();
                    
                    if (m_geometryShader) {
                        m_geometryShader->SetMatrix4("uModel", modelMatrix);
                        m_geometryShader->SetFloat("uMetallic", meshComp->GetMetallic());
                        m_geometryShader->SetFloat("uRoughness", meshComp->GetRoughness());
                    }
                    
                    meshComp->GetMesh()->Draw();
                    entityCount++;
                }
            }
        }
        Logger::Debug("DeferredRenderPipeline: Rendered " + std::to_string(entityCount) + " mesh entities from World");
    } else {
        Logger::Warning("DeferredRenderPipeline: World is null; skipping geometry draw");
    }

    const char* cap = std::getenv("GE_CAPTURE");
    if (cap && std::string(cap) == "1") {
        static bool s_dumped = false;
        if (!s_dumped) {
            int w = m_width, h = m_height;
            auto c0 = m_gBuffer->GetColorTexture(0);
            auto c1 = m_gBuffer->GetColorTexture(1);
            auto c2 = m_gBuffer->GetColorTexture(2);
            auto c3 = m_gBuffer->GetColorTexture(3);
            if (c0) FrameCapture::SaveTexturePNG(c0.get(), w, h, "/home/ubuntu/frames/gbuf0_albedo.png");
            if (c1) FrameCapture::SaveTexturePNG(c1.get(), w, h, "/home/ubuntu/frames/gbuf1_normal.png");
            if (c2) FrameCapture::SaveTexturePNG(c2.get(), w, h, "/home/ubuntu/frames/gbuf2_position.png");
            if (c3) FrameCapture::SaveTexturePNG(c3.get(), w, h, "/home/ubuntu/frames/gbuf3_misc.png");
            s_dumped = true;
        }
    }
}

void DeferredRenderPipeline::LightingPass(World* world) {
    m_lightingBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    
    glDisable(GL_DEPTH_TEST);
    
    if (!m_cachedLightManager) {
        m_cachedLightManager = std::make_unique<LightManager>();
    }
    m_cachedLightManager->CollectLights(world);
    m_cachedLightManager->ApplyBrightnessLimits();
    auto lights = m_cachedLightManager->GetActiveLights();

    const char* tcEnv = std::getenv("GE_TILED_CULL");
    bool useTiled = !(tcEnv && std::string(tcEnv) == "0");
    if (useTiled && m_tiledCullShader && m_lightGridSSBO && m_lightIndexSSBO) {
        m_tiledCullShader->Use();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightGridSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_lightIndexSSBO);
        int screenLoc = glGetUniformLocation(m_tiledCullShader->GetProgramID(), "screenSize");
        if (screenLoc >= 0) glUniform2i(screenLoc, m_width, m_height);
        int maxUpload = static_cast<int>(std::min<size_t>(lights.size(), 128));
        for (int i = 0; i < maxUpload; ++i) {
            std::string idx = std::to_string(i);
            m_tiledCullShader->SetVector3("lightPositions[" + idx + "]", lights[i]->GetPosition());
            m_tiledCullShader->SetInt("lightTypes[" + idx + "]", static_cast<int>(lights[i]->GetType()));
            m_tiledCullShader->SetFloat("lightRanges[" + idx + "]", lights[i]->GetRange());
        }
        m_tiledCullShader->SetInt("numLights", maxUpload);
        int groupsX = (m_width + 15) / 16;
        int groupsY = (m_height + 15) / 16;
        glDispatchCompute(groupsX, groupsY, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    if (!m_lightOcclusion) {
        m_lightOcclusion = std::make_unique<LightOcclusion>();
    }

    std::vector<unsigned int> headersCPU;
    std::vector<float> vertsCPU;
    int totalHeaders = 0;
    int vertFloatOffset = 0;

    for (size_t li = 0; li < lights.size(); ++li) {
        const Light* light = lights[li];
        if (!light || !light->GetCastShadows()) continue;

        m_lightOcclusion->BuildShadowVolumesForLight(light, world, static_cast<int>(li), 50.0f);
        const auto* vols = m_lightOcclusion->GetVolumesForLight(light);
        if (!vols) continue;

        for (const auto& v : *vols) {
            int vertCount = static_cast<int>(v.basePolygon.size());
            if (vertCount < 3 || v.farPolygon.size() != v.basePolygon.size()) continue;

            int baseOffset = vertFloatOffset / 4;
            for (int i = 0; i < vertCount; ++i) {
    static bool s_captured = false;
    if (!s_captured) {
        int w = m_renderData.viewportWidth;
        int h = m_renderData.viewportHeight;
        (void)w; (void)h;
        GameEngine::FrameCapture::SaveDefaultFramebufferPNG(w, h, "/home/ubuntu/frames/frame0.png");
        s_captured = true;
    }

                const Vector3& p = v.basePolygon[i];
                vertsCPU.push_back(p.x); vertsCPU.push_back(p.y); vertsCPU.push_back(p.z); vertsCPU.push_back(0.0f);
            }
            vertFloatOffset += vertCount * 4;

            int farOffset = vertFloatOffset / 4;
            for (int i = 0; i < vertCount; ++i) {
                const Vector3& p = v.farPolygon[i];
                vertsCPU.push_back(p.x); vertsCPU.push_back(p.y); vertsCPU.push_back(p.z); vertsCPU.push_back(0.0f);
            }
            vertFloatOffset += vertCount * 4;

            headersCPU.push_back(static_cast<unsigned int>(v.lightIndex < 0 ? static_cast<int>(li) : v.lightIndex));
            headersCPU.push_back(static_cast<unsigned int>(vertCount));
            headersCPU.push_back(static_cast<unsigned int>(baseOffset));
            headersCPU.push_back(static_cast<unsigned int>(farOffset));
            totalHeaders += 1;
        }
    }

    if (m_shadowVolumeHeadersSSBO == 0) {
        glGenBuffers(1, &m_shadowVolumeHeadersSSBO);
    }
    if (m_shadowVolumeVerticesSSBO == 0) {
        glGenBuffers(1, &m_shadowVolumeVerticesSSBO);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_shadowVolumeHeadersSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, headersCPU.size() * sizeof(unsigned int), headersCPU.empty() ? nullptr : headersCPU.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_shadowVolumeHeadersSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_shadowVolumeVerticesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertsCPU.size() * sizeof(float), vertsCPU.empty() ? nullptr : vertsCPU.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_shadowVolumeVerticesSSBO);

    if (m_lightingShader) {
        m_lightingShader->Use();
        m_lightingShader->SetInt("numVolumeHeaders", totalHeaders);
        m_lightingShader->SetInt("numLights", static_cast<int>(lights.size()));
        m_lightingShader->SetVector3("viewPos", Vector3(m_renderData.viewMatrix.Inverted().m[12], m_renderData.viewMatrix.Inverted().m[13], m_renderData.viewMatrix.Inverted().m[14]));
        m_lightingShader->SetInt("gAlbedoMetallic", 0);
        m_lightingShader->SetInt("gNormalRoughness", 1);
        m_lightingShader->SetInt("gPosition", 2);
        int screenLoc2 = glGetUniformLocation(m_lightingShader->GetProgramID(), "screenSize");
        if (screenLoc2 >= 0) glUniform2i(screenLoc2, m_width, m_height);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_lightGridSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_lightIndexSSBO);
        
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
            
            if (m_shadowMapBuffer) {
                auto shadowTexture = m_shadowMapBuffer->GetDepthTexture();
                if (shadowTexture) {
                    shadowTexture->Bind(3);
                    m_lightingShader->SetInt("shadowMap", 3);
                }
            }
        }
        
        if (!m_cachedLightManager) {
            m_cachedLightManager = std::make_unique<LightManager>();
        }
        m_cachedLightManager->CollectLights(world);
        m_cachedLightManager->ApplyBrightnessLimits();
        
        std::vector<LightManager::ShaderLightData> lightData;
        m_cachedLightManager->GetShaderLightData(lightData);
        
        m_lightingShader->SetInt("numLights", static_cast<int>(lightData.size()));
        
        for (size_t i = 0; i < lightData.size() && i < MAX_LIGHTS; ++i) {
            std::string indexStr = std::to_string(i);
            m_lightingShader->SetVector3("lightPositions[" + indexStr + "]", lightData[i].position);
            m_lightingShader->SetVector3("lightColors[" + indexStr + "]", lightData[i].color);
            m_lightingShader->SetFloat("lightIntensities[" + indexStr + "]", lightData[i].intensity);
            m_lightingShader->SetInt("lightTypes[" + indexStr + "]", lightData[i].type);
            m_lightingShader->SetFloat("lightRanges[" + indexStr + "]", lightData[i].range);
        }
        m_lightingShader->SetMatrix4("lightSpaceMatrix", m_lightSpaceMatrix);
        
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
