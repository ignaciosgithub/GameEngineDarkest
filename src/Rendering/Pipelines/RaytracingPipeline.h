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
    void ClearScene();
    
    void SetMaxBounces(int bounces) { m_maxBounces = bounces; }

private:
    void Cleanup();
    void SetSamplesPerPixel(int samples) { m_samplesPerPixel = samples; }

    Vector3 TraceRay(const Ray& ray, int depth = 0);
    HitInfo RayIntersectSphere(const Ray& ray, const Sphere& sphere);
    HitInfo RayIntersectScene(const Ray& ray);
    Vector3 CalculateLighting(const HitInfo& hit, const Vector3& viewDir);
    Ray GetCameraRay(float x, float y);
    
    void RenderPixel(int x, int y, std::vector<Vector3>& framebuffer);
    void RenderTile(int startX, int startY, int endX, int endY, std::vector<Vector3>& framebuffer);

    std::shared_ptr<FrameBuffer> m_framebuffer;
    std::shared_ptr<Texture> m_colorTexture;
    
    std::vector<Sphere> m_spheres;
    
    Vector3 m_cameraPos;
    Vector3 m_cameraTarget;
    Vector3 m_cameraUp;
    float m_fov;
    
    Vector3 m_lightPos;
    Vector3 m_lightColor;
    
    int m_maxBounces = 3;
    int m_samplesPerPixel = 1;
    bool m_initialized = false;
};

}
