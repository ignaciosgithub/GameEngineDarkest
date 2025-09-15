#include "LightOcclusion.h"
#include "Light.h"
#include "../Shaders/Shader.h"
#include "../../Core/ECS/World.h"
#include "../../Core/Logging/Logger.h"
#include "../../Physics/Spatial/Octree.h"
#include "../../Physics/PhysicsWorld.h"
#include "../../Physics/RigidBody/RigidBody.h"
#include "../../Core/Components/MeshComponent.h"
#include "../../Core/Components/TransformComponent.h"
#include "../Meshes/Mesh.h"
#include "../../Core/Components/RigidBodyComponent.h"
#include "../../Physics/Collision/ContinuousCollisionDetection.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {

LightOcclusion::SoftShadowMode LightOcclusion::s_defaultSoftShadowMode = LightOcclusion::SoftShadowMode::Fixed;
int LightOcclusion::s_defaultFixedSampleCount = 6;

static inline unsigned long long EdgeKey(unsigned int a, unsigned int b) {
    unsigned int x = a < b ? a : b;
    unsigned int y = a < b ? b : a;
    return (static_cast<unsigned long long>(x) << 32) | static_cast<unsigned long long>(y);
}

LightOcclusion::LightOcclusion() {
    Logger::Debug("LightOcclusion created");
    m_softShadowMode = s_defaultSoftShadowMode;
    m_fixedSampleCount = s_defaultFixedSampleCount;
    InitializeComputeShaders();
}

LightOcclusion::~LightOcclusion() {
    Shutdown();
    Logger::Debug("LightOcclusion destroyed");
}

void LightOcclusion::Initialize(PhysicsWorld* physicsWorld) {
    if (!physicsWorld) {
        Logger::Error("LightOcclusion::Initialize - PhysicsWorld is null");
        return;
    }
    
    m_physicsWorld = physicsWorld;
    Logger::Info("LightOcclusion initialized with PhysicsWorld");
}

void LightOcclusion::Shutdown() {
    m_physicsWorld = nullptr;
    Logger::Debug("LightOcclusion shutdown complete");
}

float LightOcclusion::CalculateOcclusion(const Light* light, const Vector3& targetPoint, World* world) {
    if (!m_occlusionEnabled || !light || !world || !m_physicsWorld) {
        return 1.0f;
    }

    Vector3 lightPos;
    Vector3 lightDir;
    float maxDist = m_maxOcclusionDistance;

    switch (light->GetType()) {
        case LightType::Directional: {
            lightDir = light->GetDirection().Normalized();
            lightPos = targetPoint - lightDir * maxDist;
            break;
        }
        case LightType::Point:
        case LightType::Spot: {
            lightPos = light->GetPosition();
            lightDir = (targetPoint - lightPos).Normalized();
            maxDist = (targetPoint - lightPos).Length();
            if (maxDist > light->GetRange()) {
                return 0.0f;
            }
            break;
        }
        default:
            lightPos = targetPoint;
            lightDir = Vector3::Zero;
            break;
    }

    if (light->GetType() == LightType::Spot) {
        Vector3 spotDir = light->GetDirection().Normalized();
        float d = std::max(-1.0f, std::min(1.0f, lightDir.Dot(spotDir)));
        float angle = std::acos(d);
        float outer = light->GetOuterConeAngle() * (3.14159265f / 180.0f);
        if (angle > outer) {
            return 0.0f;
        }
    }

    if (m_shadowSoftness > 0.0f) {
        return CalculateSoftShadowOcclusion(light, targetPoint, world);
    }

    OcclusionRayHit hit;
    if (RaycastForOcclusion(lightPos, targetPoint, hit)) {
        if (hit.distance < maxDist - 0.01f) {
            return 0.0f;
        }
    }

    return 1.0f;
}

bool LightOcclusion::RaycastForOcclusion(const Vector3& start, const Vector3& end, OcclusionRayHit& hit) {
    if (!m_physicsWorld) {
        return false;
    }

    hit.hit = false;
    float totalLen = (end - start).Length();
    hit.distance = totalLen;

    auto occludingBodies = GetOccludingBodiesForSegment(start, end);

    float closestDistance = totalLen;
    bool foundHit = false;

    for (RigidBody* body : occludingBodies) {
        if (!body || !IsBodyOccluding(body)) continue;

        ContinuousCollisionInfo collisionInfo;
        if (ContinuousCollisionDetection::RaycastAgainstBody(start, end, body, collisionInfo)) {
            float hitDistance = std::max(0.0f, std::min(totalLen, collisionInfo.timeOfImpact * totalLen));
            if (hitDistance < closestDistance) {
                closestDistance = hitDistance;
                hit.hit = true;
                hit.hitPoint = collisionInfo.contactPoint;
                hit.hitNormal = collisionInfo.normal;
                hit.distance = hitDistance;
                hit.hitBody = body;
                foundHit = true;
            }
        }
    }

    return foundHit;
}

bool LightOcclusion::IsLightOccluded(const Light* light, const Vector3& targetPoint, World* world) {
    float occlusion = CalculateOcclusion(light, targetPoint, world);
    return occlusion < 0.5f; // Consider occluded if less than 50% light reaches target
}

float LightOcclusion::CalculateShadowAttenuation(const Light* light, const Vector3& targetPoint, World* world) {
    if (!m_occlusionEnabled) {
        return 1.0f; // No shadow attenuation
    }
    
    float occlusion = CalculateOcclusion(light, targetPoint, world);
    
    if (light->GetType() == LightType::Point || light->GetType() == LightType::Spot) {
        float distance = (targetPoint - light->GetPosition()).Length();
        float distanceAttenuation = CalculateDistanceAttenuation(distance, light->GetRange());
        occlusion *= distanceAttenuation;
    }
    
    return occlusion;
}

std::vector<RigidBody*> LightOcclusion::GetOccludingBodies(World* /* world */) {
    std::vector<RigidBody*> occludingBodies;
    if (!m_physicsWorld) return occludingBodies;
    const auto& bodies = m_physicsWorld->GetRigidBodies();

    occludingBodies.reserve(bodies.size());
    for (RigidBody* b : bodies) {
        if (b && IsBodyOccluding(b)) occludingBodies.push_back(b);
    }
    return occludingBodies;
}

bool LightOcclusion::IsBodyOccluding(RigidBody* body) {
    if (!body) return false;
    
    return body->IsStatic();
}

float LightOcclusion::CalculateDistanceAttenuation(float distance, float maxDistance) {
    if (distance >= maxDistance) {
        return 0.0f;
    }
    
    float attenuation = 1.0f - (distance * distance) / (maxDistance * maxDistance);
    return std::max(0.0f, attenuation);
}

float LightOcclusion::CalculateSoftShadowOcclusion(const Light* light, const Vector3& targetPoint, World* /* world */) {
    int sampleCount = m_fixedSampleCount;
    if (m_softShadowMode == SoftShadowMode::Off) {
        sampleCount = 1;
    } else if (m_softShadowMode == SoftShadowMode::Adaptive) {
        sampleCount = std::max(4, std::min(16, m_fixedSampleCount));
    }
    Vector3 baseLightPos;
    Vector3 baseDir;
    float baseMaxDist = m_maxOcclusionDistance;

    if (light->GetType() == LightType::Directional) {
        baseDir = light->GetDirection().Normalized();
        baseLightPos = targetPoint - baseDir * baseMaxDist;
    } else {
        baseLightPos = light->GetPosition();
        baseDir = (targetPoint - baseLightPos).Normalized();
        baseMaxDist = (targetPoint - baseLightPos).Length();
        if (baseMaxDist > light->GetRange()) return 0.0f;
    }

    std::vector<Vector3> samplePoints = GenerateSamplePoints(baseLightPos, targetPoint, sampleCount);

    int litSamples = 0;
    for (const Vector3& sp : samplePoints) {
        OcclusionRayHit hit;
        bool blocked = false;
        if (RaycastForOcclusion(sp, targetPoint, hit)) {
            float d = (targetPoint - sp).Length();
            if (hit.distance < d - 0.01f) blocked = true;
        }
        if (!blocked) ++litSamples;
    }

    float visibility = (float)litSamples / (float)std::max(1, sampleCount);

    if (light->GetType() == LightType::Point || light->GetType() == LightType::Spot) {
        float dist = (targetPoint - light->GetPosition()).Length();
        visibility *= CalculateDistanceAttenuation(dist, light->GetRange());
    }

    if (light->GetType() == LightType::Spot) {
        Vector3 spotDir = light->GetDirection().Normalized();
        Vector3 L = (targetPoint - light->GetPosition()).Normalized();
        float d = std::max(-1.0f, std::min(1.0f, L.Dot(spotDir)));
        float angle = std::acos(d);
        float inner = light->GetInnerConeAngle() * (3.14159265f / 180.0f);
        float outer = light->GetOuterConeAngle() * (3.14159265f / 180.0f);
        if (angle > outer) {
            visibility = 0.0f;
        } else if (angle > inner) {
            float t = (angle - inner) / std::max(1e-4f, (outer - inner));
            visibility *= (1.0f - t);
        }
    }

    return visibility;
}

std::vector<Vector3> LightOcclusion::GenerateSamplePoints(const Vector3& lightPos, const Vector3& targetPoint, int sampleCount) {
    std::vector<Vector3> samplePoints;
    samplePoints.reserve(sampleCount);
    samplePoints.push_back(lightPos);
    if (sampleCount <= 1) {
        return samplePoints;
    }
    Vector3 toLightDir = (lightPos - targetPoint).Normalized();
    Vector3 perpendicular1 = toLightDir.Cross(Vector3::Up).Normalized();
    Vector3 perpendicular2 = toLightDir.Cross(perpendicular1).Normalized();
    float sampleRadius = m_shadowSoftness;
    for (int i = 1; i < sampleCount; ++i) {
        float angle = (2.0f * 3.14159f * i) / (sampleCount - 1);
        float radius = sampleRadius * std::sqrt(static_cast<float>(i) / (sampleCount - 1));
        Vector3 offset = perpendicular1 * (radius * std::cos(angle)) + perpendicular2 * (radius * std::sin(angle));
        samplePoints.push_back(lightPos + offset);
    }
    return samplePoints;
}

const LightOcclusion::MeshAdjacency& LightOcclusion::GetOrBuildAdjacency(const Mesh* mesh) {
    auto it = m_adjacencyCache.find(mesh);
    if (it != m_adjacencyCache.end() && it->second.built) return it->second;
    MeshAdjacency adj;
    const auto& verts = mesh->GetVertices();
    adj.positions.reserve(verts.size());
    for (const auto& v : verts) adj.positions.push_back(v.position);
    adj.indices = mesh->GetIndices();
    for (size_t tri = 0; tri + 2 < adj.indices.size(); tri += 3) {
        unsigned int i0 = adj.indices[tri];
        unsigned int i1 = adj.indices[tri + 1];
        unsigned int i2 = adj.indices[tri + 2];
        unsigned long long e01 = EdgeKey(i0, i1);
        unsigned long long e12 = EdgeKey(i1, i2);
        unsigned long long e20 = EdgeKey(i2, i0);
        adj.edgeFaceCount[e01] += 1;
        adj.edgeFaceCount[e12] += 1;
        adj.edgeFaceCount[e20] += 1;
        adj.edgeToTris[e01].push_back((unsigned int)tri);
        adj.edgeToTris[e12].push_back((unsigned int)tri);
        adj.edgeToTris[e20].push_back((unsigned int)tri);
    }
    adj.built = true;
    auto ins = m_adjacencyCache.emplace(mesh, std::move(adj));
    return ins.first->second;
}

void LightOcclusion::CollectBoundaryVerticesForLight(const Light* light, World* world, std::vector<ShadowVertex>& out) {
    out.clear();
    if (!light || !world) return;

    Vector3 lightPos = light->GetPosition();
    Vector3 lightDir = light->GetDirection().Normalized();
    
    auto entities = world->GetEntities();
    auto workerCount = std::max(1u, std::thread::hardware_concurrency());
    
    if (workerCount <= 1 || entities.size() < 10) {
        ProcessEntitiesForShadowVertices(entities.begin(), entities.end(), light, world, lightPos, lightDir, out);
        return;
    }
    
    std::mutex resultMutex;
    std::vector<std::thread> threads;
    size_t entitiesPerThread = std::max<size_t>(1, entities.size() / workerCount);
    
    for (unsigned t = 0; t < workerCount; ++t) {
        auto start = entities.begin() + (t * entitiesPerThread);
        auto end = (t == workerCount - 1) ? entities.end() : start + entitiesPerThread;
        
        if (start >= entities.end()) break;
        
        threads.emplace_back([&, start, end]() {
            std::vector<ShadowVertex> localResults;
            ProcessEntitiesForShadowVertices(start, end, light, world, lightPos, lightDir, localResults);
            
            std::lock_guard<std::mutex> lock(resultMutex);
            out.insert(out.end(), localResults.begin(), localResults.end());
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

void LightOcclusion::BuildAreasFromBoundaryVertices(const Light* , const std::vector<ShadowVertex>& seeds, std::vector<ShadowArea>& areas) {
    areas.clear();
    if (seeds.size() < 3) return;
    Vector3 centroid = Vector3::Zero;
    for (const auto& s : seeds) centroid += s.positionWS;
    centroid /= static_cast<float>(seeds.size());
    std::vector<std::pair<float, ShadowVertex>> sorted;
    sorted.reserve(seeds.size());
    Vector3 ref = seeds[0].positionWS - centroid;
    Vector3 up = Vector3::Up;
    for (const auto& s : seeds) {
        Vector3 d = s.positionWS - centroid;
        float ang = atan2f(ref.Cross(d).Dot(up), ref.Dot(d));
        sorted.emplace_back(ang, s);
    }
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b){ return a.first < b.first; });
    ShadowArea area;
    for (const auto& pr : sorted) area.vertices.push_back(pr.second);
    if (area.vertices.size() >= 3) areas.push_back(std::move(area));
}

void LightOcclusion::ExtrudeAreasToVolumes(const Light* light, const std::vector<ShadowArea>& areas, std::vector<ShadowVolume>& volumes, float dirFar) {
    volumes.clear();
    for (const auto& a : areas) {
        if (a.vertices.size() < 3) continue;
        ShadowVolume vol;
        vol.basePolygon.reserve(a.vertices.size());
        vol.farPolygon.reserve(a.vertices.size());
        for (const auto& sv : a.vertices) {
            vol.basePolygon.push_back(sv.positionWS);
            float fd = (light->GetType() == LightType::Directional) ? dirFar : light->GetRange();
            if (light->GetType() == LightType::Spot) {
                Vector3 L = (sv.positionWS - light->GetPosition()).Normalized();
                float d = std::max(-1.0f, std::min(1.0f, L.Dot(light->GetDirection().Normalized())));
                float ang = std::acos(d);
                float outer = light->GetOuterConeAngle() * (3.14159265f / 180.0f);
                if (ang > outer) continue;
            }
            vol.farPolygon.push_back(sv.positionWS + sv.dirFromLight * fd);
        }
        if (vol.basePolygon.size() >= 3 && vol.farPolygon.size() == vol.basePolygon.size()) {
            volumes.push_back(std::move(vol));
        }
    }
}

void LightOcclusion::BuildShadowVolumesForLight(const Light* light, World* world, int lightIndex, float dirFar) {
    if (m_useGPUCompute) {
        BuildShadowVolumesGPU(light, world, lightIndex, dirFar);
        return;
    }
    if (!light || !world) return;
    
    if (!ShouldRebuildShadowVolumes(light, world)) {
        return;
    }
    
    if (m_useGPUCompute && m_shadowVolumeGenShader && m_shadowVolumeExtrudeShader) {
        BuildShadowVolumesGPU(light, world, lightIndex, dirFar);
    } else {
        std::vector<ShadowVertex> boundary;
        CollectBoundaryVerticesForLight(light, world, boundary);
        std::vector<ShadowArea> areas;
        BuildAreasFromBoundaryVertices(light, boundary, areas);
        std::vector<ShadowVolume> vols;
        ExtrudeAreasToVolumes(light, areas, vols, dirFar);
        for (auto& v : vols) v.lightIndex = lightIndex;
        m_lightVolumes[light] = std::move(vols);
    }
    
    m_lightVersions[light] = ++m_worldVersion;
}

const std::vector<ShadowVolume>* LightOcclusion::GetVolumesForLight(const Light* light) const {
    auto it = m_lightVolumes.find(light);
    if (it == m_lightVolumes.end()) return nullptr;
    return &it->second;
}

std::vector<RigidBody*> LightOcclusion::GetOccludingBodiesForSegment(const Vector3& start, const Vector3& end) {
    std::vector<RigidBody*> result;
    if (!m_physicsWorld) return result;
    const Octree* oct = m_physicsWorld->GetOctree();
    if (!oct) {
        const auto& bodies = m_physicsWorld->GetRigidBodies();
        result.reserve(bodies.size());
        for (RigidBody* b : bodies) {
            if (b && IsBodyOccluding(b)) result.push_back(b);
        }
        return result;
    }
    Vector3 segMin(std::min(start.x, end.x), std::min(start.y, end.y), std::min(start.z, end.z));
    Vector3 segMax(std::max(start.x, end.x), std::max(start.y, end.y), std::max(start.z, end.z));
    const float pad = 0.05f;
    segMin = segMin - Vector3(pad, pad, pad);
    segMax = segMax + Vector3(pad, pad, pad);
    AABB query(segMin, segMax);
    std::vector<RigidBody*> candidates;
    oct->Query(query, candidates);
    result.reserve(candidates.size());
    for (RigidBody* b : candidates) {
        if (b && IsBodyOccluding(b)) result.push_back(b);
    }
    return result;
}

template<typename Iterator>
void LightOcclusion::ProcessEntitiesForShadowVertices(Iterator start, Iterator end, const Light* light, World* world, 
                                                    const Vector3& lightPos, const Vector3& lightDir, std::vector<ShadowVertex>& out) {
    for (auto it = start; it != end; ++it) {
        const auto& e = *it;
        auto* tc = world->GetComponent<TransformComponent>(e);
        auto* mc = world->GetComponent<MeshComponent>(e);
        if (!tc || !mc || !mc->HasMesh()) continue;

        const Mesh* mesh = mc->GetMesh().get();
        const auto& adj = GetOrBuildAdjacency(mesh);
        const auto& idx = adj.indices;
        const auto& pos = adj.positions;

        Matrix4 model = tc->transform.GetLocalToWorldMatrix();

        auto faceFacing = [&](unsigned int triStart)->float {
            unsigned int i0 = idx[triStart], i1 = idx[triStart + 1], i2 = idx[triStart + 2];
            Vector3 p0 = model * pos[i0];
            Vector3 p1 = model * pos[i1];
            Vector3 p2 = model * pos[i2];
            Vector3 n = (p1 - p0).Cross(p2 - p0).Normalized();
            if (light->GetType() == LightType::Directional) {
                return n.Dot(-lightDir.Normalized());
            } else {
                Vector3 lightToVertex = (p0 - lightPos);
                if (lightToVertex.LengthSquared() > 1e-6f) {
                    return n.Dot(lightToVertex.Normalized());
                }
                return 0.0f;
            }
        };

        auto emitEdge = [&](const Vector3& a, const Vector3& b) {
            ShadowVertex svA; svA.positionWS = a;
            svA.dirFromLight = (light->GetType() == LightType::Directional) ? -lightDir : (a - lightPos).Normalized();
            ShadowVertex svB; svB.positionWS = b;
            svB.dirFromLight = (light->GetType() == LightType::Directional) ? -lightDir : (b - lightPos).Normalized();
            out.push_back(svA);
            out.push_back(svB);
        };

        for (size_t tri = 0; tri + 2 < idx.size(); tri += 3) {
            unsigned int i0 = idx[tri], i1 = idx[tri + 1], i2 = idx[tri + 2];
            float fThis = faceFacing((unsigned int)tri);
            if (fThis <= 1e-4f) continue;

            Vector3 p0 = model * pos[i0];
            Vector3 p1 = model * pos[i1];
            Vector3 p2 = model * pos[i2];

            auto handleEdge = [&](unsigned int a, unsigned int b, const Vector3& pa, const Vector3& pb) {
                unsigned long long ek = EdgeKey(a, b);
                auto fcIt = adj.edgeFaceCount.find(ek);
                int count = (fcIt != adj.edgeFaceCount.end()) ? fcIt->second : 0;
                bool boundary = count <= 1;

                bool opposite = false;
                if (!boundary) {
                    auto adjIt = adj.edgeToTris.find(ek);
                    if (adjIt != adj.edgeToTris.end()) {
                        for (unsigned int triIdx : adjIt->second) {
                            if (triIdx == tri) continue;
                            float fOther = faceFacing(triIdx);
                            if ((fThis > 1e-4f) != (fOther > 1e-4f)) {
                                opposite = true;
                                break;
                            }
                        }
                    }
                }

                if (boundary || opposite) {
                    emitEdge(pa, pb);
                }
            };

            handleEdge(i0, i1, p0, p1);
            handleEdge(i1, i2, p1, p2);
            handleEdge(i2, i0, p2, p0);
        }
    }
}

void LightOcclusion::InitializeComputeShaders() {
    const char* gpuVol = std::getenv("GE_GPU_SHADOW_VOL");
    m_useGPUCompute = !(gpuVol && std::string(gpuVol) == "0");
    if (!m_useGPUCompute) {
        Logger::Info("GPU shadow volumes disabled by GE_GPU_SHADOW_VOL=0");
        return;
    }

    if (!m_shadowVolumeGenShader) {
        m_shadowVolumeGenShader = std::make_shared<Shader>();
        if (!m_shadowVolumeGenShader->LoadComputeShader("src/Rendering/Shaders/shadow_volume_generation.comp")) {
            Logger::Warning("Failed to load shadow volume generation compute shader, falling back to CPU");
            m_useGPUCompute = false;
            return;
        }
    }
    
    if (!m_shadowVolumeExtrudeShader) {
        m_shadowVolumeExtrudeShader = std::make_shared<Shader>();
        if (!m_shadowVolumeExtrudeShader->LoadComputeShader("src/Rendering/Shaders/shadow_volume_extrusion.comp")) {
            Logger::Warning("Failed to load shadow volume extrusion compute shader, falling back to CPU");
            m_useGPUCompute = false;
            return;
        }
    }
    
    Logger::Info("Shadow volume compute shaders initialized successfully");
}

void LightOcclusion::BuildShadowVolumesGPU(const Light* light, World* world, int lightIndex, float dirFar) {
    Logger::Info("Building shadow volumes using GPU compute shaders (simplified implementation)");
    
    std::vector<ShadowVertex> boundary;
    CollectBoundaryVerticesForLight(light, world, boundary);
    std::vector<ShadowArea> areas;
    BuildAreasFromBoundaryVertices(light, boundary, areas);
    std::vector<ShadowVolume> vols;
    ExtrudeAreasToVolumes(light, areas, vols, dirFar);
    for (auto& v : vols) v.lightIndex = lightIndex;
    m_lightVolumes[light] = std::move(vols);
}

bool LightOcclusion::ShouldRebuildShadowVolumes(const Light* light, World* world) const {
    (void)world; // Mark parameter as intentionally unused for now
    
    auto it = m_lightVersions.find(light);
    if (it == m_lightVersions.end()) {
        return true;
    }
    
    return it->second != m_worldVersion;
}

void LightOcclusion::MarkShadowVolumesDirty(const Light* light) {
    m_lightVersions.erase(light);
    ++m_worldVersion;
}

}
