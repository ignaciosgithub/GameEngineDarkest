#pragma once

#include "RenderPipeline.h"
#include "../Core/Texture.h"
#include "../Core/FrameBuffer.h"
#include "../Shaders/Shader.h"
#include "../../Core/Math/Vector3.h"
#include <memory>
#include <vector>

namespace GameEngine {

struct Ray {
    Vector3 origin;
    Vector3 direction;
    
    Ray(const Vector3& o, const Vector3& d) : origin(o), direction(d) {}
};

struct Triangle {
    Vector3 v0, v1, v2;
    Vector3 normal;
    Vector3 color;
    float reflectivity;
    
    Triangle() = default;
    Triangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, 
             const Vector3& col, float refl = 0.0f) 
        : v0(vertex0), v1(vertex1), v2(vertex2), color(col), reflectivity(refl) {
        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        normal = edge1.Cross(edge2).Normalized();
    }
};

struct BVHNode {
    Vector3 minBounds;
    Vector3 maxBounds;
    int leftChild = -1;
    int rightChild = -1;
    int triangleStart = -1;
    int triangleCount = 0;
    
    BVHNode() = default;
};

struct Sphere {
    Vector3 center;
    float radius;
    Vector3 color;
    float reflectivity;
    
    Sphere(const Vector3& c, float r, const Vector3& col, float refl = 0.0f) 
        : center(c), radius(r), color(col), reflectivity(refl) {}
};

struct HitInfo {
    bool hit = false;
    float distance = 0.0f;
    Vector3 point;
    Vector3 normal;
    Vector3 color;
    float reflectivity = 0.0f;
};

class RaytracingPipeline : public RenderPipeline {
public:
    RaytracingPipeline();
    ~RaytracingPipeline() override;

    bool Initialize(int width, int height) override;
    void Shutdown() override;
    void BeginFrame(const RenderData& renderData) override;
    void Render(World* world) override;
    void EndFrame() override;
    void Resize(int width, int height) override;
    
    std::shared_ptr<Texture> GetFinalTexture() const override;

    void AddSphere(const Sphere& sphere);
    void AddTriangle(const Triangle& triangle);
    void ClearScene();
    void BuildBVH();
    
    void SetMaxBounces(int bounces) { m_maxBounces = bounces; }
    void SetUseComputeShader(bool use) { m_useComputeShader = use; }

private:
    void Cleanup();
    void SetSamplesPerPixel(int samples) { m_samplesPerPixel = samples; }

    Vector3 TraceRay(const Ray& ray, int depth = 0);
    HitInfo RayIntersectSphere(const Ray& ray, const Sphere& sphere);
    HitInfo RayIntersectTriangle(const Ray& ray, const Triangle& triangle);
    HitInfo RayIntersectScene(const Ray& ray);
    Vector3 CalculateLighting(const HitInfo& hit, const Vector3& viewDir);
    Ray GetCameraRay(float x, float y);
    
    void RenderPixel(int x, int y, std::vector<Vector3>& framebuffer);
    void RenderTile(int startX, int startY, int endX, int endY, std::vector<Vector3>& framebuffer);
    
    void BuildBVHRecursive(int nodeIndex, std::vector<int>& triangleIndices);
    bool RayBoxIntersect(const Ray& ray, const Vector3& boxMin, const Vector3& boxMax);
    HitInfo TraverseBVH(const Ray& ray);
    void RenderWithComputeShader();
    void SetupComputeShaderBuffers();

    std::shared_ptr<FrameBuffer> m_framebuffer;
    std::shared_ptr<Texture> m_colorTexture;
    
    std::vector<Sphere> m_spheres;
    std::vector<Triangle> m_triangles;
    std::vector<BVHNode> m_bvhNodes;
    
    Vector3 m_cameraPos;
    Vector3 m_cameraTarget;
    Vector3 m_cameraUp;
    float m_fov;
    
    Vector3 m_lightPos;
    Vector3 m_lightColor;
    
    int m_maxBounces = 3;
    int m_samplesPerPixel = 1;
    bool m_initialized = false;
    bool m_useComputeShader = true;
    
    std::shared_ptr<Shader> m_computeShader;
    unsigned int m_triangleSSBO = 0;
    unsigned int m_bvhSSBO = 0;
};

}
