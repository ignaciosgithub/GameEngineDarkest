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
    LightOcclusion();
    ~LightOcclusion();
    
    // Initialize with physics world for raycasting
    void Initialize(PhysicsWorld* physicsWorld);
    void Shutdown();
    
    // Main occlusion calculation
    float CalculateOcclusion(const Light* light, const Vector3& targetPoint, World* world);
    
    // Raycast against physics bodies for occlusion
    bool RaycastForOcclusion(const Vector3& start, const Vector3& end, OcclusionRayHit& hit);
    
    // Check if a light is occluded by walls
    bool IsLightOccluded(const Light* light, const Vector3& targetPoint, World* world);
    
    // Calculate shadow attenuation based on occlusion
    float CalculateShadowAttenuation(const Light* light, const Vector3& targetPoint, World* world);
    
    // Settings
    void SetOcclusionEnabled(bool enabled) { m_occlusionEnabled = enabled; }
    bool IsOcclusionEnabled() const { return m_occlusionEnabled; }
    
    void SetShadowSoftness(float softness) { m_shadowSoftness = softness; }
    float GetShadowSoftness() const { return m_shadowSoftness; }
    
    void SetMaxOcclusionDistance(float distance) { m_maxOcclusionDistance = distance; }
    float GetMaxOcclusionDistance() const { return m_maxOcclusionDistance; }
    
private:
    PhysicsWorld* m_physicsWorld = nullptr;
    bool m_occlusionEnabled = true;
    float m_shadowSoftness = 0.1f;
    float m_maxOcclusionDistance = 100.0f;
    
    // Helper methods
    std::vector<RigidBody*> GetOccludingBodies(World* world);
    bool IsBodyOccluding(RigidBody* body);
    float CalculateDistanceAttenuation(float distance, float maxDistance);
    
    // Multi-sample occlusion for soft shadows
    float CalculateSoftShadowOcclusion(const Light* light, const Vector3& targetPoint, World* world);
    // Pruned occluder collection using segment AABB and octree when available
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
    };
    std::unordered_map<const Mesh*, MeshAdjacency> m_adjacencyCache;
    std::unordered_map<const Light*, std::vector<ShadowVolume>> m_lightVolumes;

    const MeshAdjacency& GetOrBuildAdjacency(const Mesh* mesh);
    void CollectBoundaryVerticesForLight(const Light* light, World* world, std::vector<ShadowVertex>& out);
    void BuildAreasFromBoundaryVertices(const Light* light, const std::vector<ShadowVertex>& seeds, std::vector<ShadowArea>& areas);
    void ExtrudeAreasToVolumes(const Light* light, const std::vector<ShadowArea>& areas, std::vector<ShadowVolume>& volumes, float dirFar);


    std::vector<Vector3> GenerateSamplePoints(const Vector3& lightPos, const Vector3& targetPoint, int sampleCount);
    
    // GPU compute shader support
    void InitializeComputeShaders();
    void BuildShadowVolumesGPU(const Light* light, World* world, int lightIndex, float dirFar);
    bool m_useGPUCompute = true;
    std::shared_ptr<class Shader> m_shadowVolumeGenShader;
    std::shared_ptr<class Shader> m_shadowVolumeExtrudeShader;
    
    // GPU buffer objects
    unsigned int m_meshVerticesSSBO = 0;
    unsigned int m_meshIndicesSSBO = 0;
    unsigned int m_meshTransformsSSBO = 0;
    unsigned int m_shadowVerticesSSBO = 0;
    unsigned int m_shadowDirectionsSSBO = 0;
    unsigned int m_outputCountersSSBO = 0;
    
    // Threading support for CPU fallback
    template<typename Iterator>
    void ProcessEntitiesForShadowVertices(Iterator start, Iterator end, const Light* light, World* world, 
                                        const Vector3& lightPos, const Vector3& lightDir, std::vector<ShadowVertex>& out);
    
    // Change detection for caching
    bool ShouldRebuildShadowVolumes(const Light* light, World* world) const;
    void MarkShadowVolumesDirty(const Light* light);
    mutable std::unordered_map<const Light*, uint64_t> m_lightVersions;
    mutable uint64_t m_worldVersion = 0;
};

}
