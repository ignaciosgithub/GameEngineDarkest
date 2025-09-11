#pragma once

#include "../../Core/Math/Vector3.h"
#include "../../Physics/PhysicsWorld.h"
#include "../../Physics/RigidBody/RigidBody.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

namespace GameEngine {

class Mesh;

class Light;
class World;

struct OcclusionRayHit {
    bool hit = false;
    Vector3 hitPoint;
    Vector3 hitNormal;
    float distance = 0.0f;
    RigidBody* hitBody = nullptr;
};

struct ShadowVertex {
    Vector3 positionWS;
    Vector3 dirFromLight;
};

struct ShadowArea {
    std::vector<ShadowVertex> vertices;
};

struct ShadowVolume {
    std::vector<Vector3> basePolygon;
    std::vector<Vector3> farPolygon;
    int lightIndex = -1;
};

class LightOcclusion {
public:
    enum class SoftShadowMode { Off = 0, Fixed = 1, Adaptive = 2 };

    LightOcclusion();
    ~LightOcclusion();
    
    void Initialize(PhysicsWorld* physicsWorld);
    void Shutdown();
    
    float CalculateOcclusion(const Light* light, const Vector3& targetPoint, World* world);
    bool RaycastForOcclusion(const Vector3& start, const Vector3& end, OcclusionRayHit& hit);
    bool IsLightOccluded(const Light* light, const Vector3& targetPoint, World* world);
    float CalculateShadowAttenuation(const Light* light, const Vector3& targetPoint, World* world);
    
    void SetOcclusionEnabled(bool enabled) { m_occlusionEnabled = enabled; }
    bool IsOcclusionEnabled() const { return m_occlusionEnabled; }
    
    void SetShadowSoftness(float softness) { m_shadowSoftness = softness; }
    float GetShadowSoftness() const { return m_shadowSoftness; }
    
    void SetMaxOcclusionDistance(float distance) { m_maxOcclusionDistance = distance; }
    float GetMaxOcclusionDistance() const { return m_maxOcclusionDistance; }

    void SetSoftShadowMode(SoftShadowMode mode) { m_softShadowMode = mode; }
    void SetFixedSampleCount(int count) { m_fixedSampleCount = count; }

    static void SetDefaultSoftShadowMode(SoftShadowMode mode) { s_defaultSoftShadowMode = mode; }
    static void SetDefaultFixedSampleCount(int count) { s_defaultFixedSampleCount = count; }
    static SoftShadowMode GetDefaultSoftShadowMode() { return s_defaultSoftShadowMode; }
    static int GetDefaultFixedSampleCount() { return s_defaultFixedSampleCount; }
    
public:
    void BuildShadowVolumesForLight(const Light* light, World* world, int lightIndex, float dirFar);
    const std::vector<ShadowVolume>* GetVolumesForLight(const Light* light) const;
    std::vector<RigidBody*> GetOccludingBodiesForSegment(const Vector3& start, const Vector3& end);
private:
    struct MeshAdjacency {
        bool built = false;
        std::vector<unsigned int> indices;
        std::vector<Vector3> positions;
        std::unordered_map<unsigned long long, int> edgeFaceCount;
        std::unordered_map<unsigned long long, std::vector<unsigned int>> edgeToTris;
    };
    std::unordered_map<const Mesh*, MeshAdjacency> m_adjacencyCache;
    std::unordered_map<const Light*, std::vector<ShadowVolume>> m_lightVolumes;

    const MeshAdjacency& GetOrBuildAdjacency(const Mesh* mesh);
    void CollectBoundaryVerticesForLight(const Light* light, World* world, std::vector<ShadowVertex>& out);
    void BuildAreasFromBoundaryVertices(const Light* light, const std::vector<ShadowVertex>& seeds, std::vector<ShadowArea>& areas);
    void ExtrudeAreasToVolumes(const Light* light, const std::vector<ShadowArea>& areas, std::vector<ShadowVolume>& volumes, float dirFar);

    std::vector<RigidBody*> GetOccludingBodies(World* world);
    bool IsBodyOccluding(RigidBody* body);
    float CalculateDistanceAttenuation(float distance, float maxDistance);

    float CalculateSoftShadowOcclusion(const Light* light, const Vector3& targetPoint, World* world);
    std::vector<Vector3> GenerateSamplePoints(const Vector3& lightPos, const Vector3& targetPoint, int sampleCount);
    
    void InitializeComputeShaders();
    void BuildShadowVolumesGPU(const Light* light, World* world, int lightIndex, float dirFar);
    bool m_useGPUCompute = false;
    std::shared_ptr<class Shader> m_shadowVolumeGenShader;
    std::shared_ptr<class Shader> m_shadowVolumeExtrudeShader;
    
    unsigned int m_meshVerticesSSBO = 0;
    unsigned int m_meshIndicesSSBO = 0;
    unsigned int m_meshTransformsSSBO = 0;
    unsigned int m_shadowVerticesSSBO = 0;
    unsigned int m_shadowDirectionsSSBO = 0;
    unsigned int m_outputCountersSSBO = 0;
    
    template<typename Iterator>
    void ProcessEntitiesForShadowVertices(Iterator start, Iterator end, const Light* light, World* world, 
                                        const Vector3& lightPos, const Vector3& lightDir, std::vector<ShadowVertex>& out);
    
    bool ShouldRebuildShadowVolumes(const Light* light, World* world) const;
    void MarkShadowVolumesDirty(const Light* light);
    mutable std::unordered_map<const Light*, uint64_t> m_lightVersions;
    mutable uint64_t m_worldVersion = 0;

    PhysicsWorld* m_physicsWorld = nullptr;
    bool m_occlusionEnabled = true;
    float m_shadowSoftness = 0.1f;
    float m_maxOcclusionDistance = 100.0f;
    SoftShadowMode m_softShadowMode = SoftShadowMode::Off;
    int m_fixedSampleCount = 6;

    static SoftShadowMode s_defaultSoftShadowMode;
    static int s_defaultFixedSampleCount;
};

}
