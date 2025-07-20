#include "RaytracingPipeline.h"
#include "../../Core/Logging/Logger.h"
#include "../../Core/ECS/World.h"
#include <GL/gl.h>
#include <cmath>
#include <algorithm>
#include <random>

namespace GameEngine {

RaytracingPipeline::RaytracingPipeline() 
    : m_cameraPos(0.0f, 0.0f, 3.0f)
    , m_cameraTarget(0.0f, 0.0f, 0.0f)
    , m_cameraUp(0.0f, 1.0f, 0.0f)
    , m_fov(45.0f)
    , m_lightPos(5.0f, 5.0f, 5.0f)
    , m_lightColor(1.0f, 1.0f, 1.0f) {
}

RaytracingPipeline::~RaytracingPipeline() {
    Cleanup();
}

bool RaytracingPipeline::Initialize(int width, int height) {
    if (m_initialized) {
        return true;
    }
    
    m_renderData.viewportWidth = width;
    m_renderData.viewportHeight = height;
    
    m_framebuffer = std::make_shared<FrameBuffer>(width, height);
    
    m_colorTexture = std::make_shared<Texture>();
    m_colorTexture->CreateEmpty(width, height, TextureFormat::RGBA8);
    
    m_framebuffer->AddColorAttachment(TextureFormat::RGBA8);
    
    if (!m_framebuffer->IsComplete()) {
        Logger::Error("Raytracing framebuffer is not complete");
        return false;
    }
    
    AddSphere(Sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f, Vector3(1.0f, 0.2f, 0.2f), 0.3f));
    AddSphere(Sphere(Vector3(-2.0f, 0.0f, -1.0f), 0.8f, Vector3(0.2f, 1.0f, 0.2f), 0.1f));
    AddSphere(Sphere(Vector3(2.0f, 0.0f, -1.0f), 0.6f, Vector3(0.2f, 0.2f, 1.0f), 0.8f));
    AddSphere(Sphere(Vector3(0.0f, -100.5f, -1.0f), 100.0f, Vector3(0.8f, 0.8f, 0.8f), 0.0f));
    
    m_initialized = true;
    Logger::Info("Raytracing pipeline initialized with " + std::to_string(m_spheres.size()) + " spheres");
    return true;
}

void RaytracingPipeline::Render(World* /*world*/) {
    if (!m_initialized) {
        return;
    }
    
    m_framebuffer->Bind();
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    int width = m_renderData.viewportWidth;
    int height = m_renderData.viewportHeight;
    
    std::vector<Vector3> framebuffer(width * height);
    
    const int tileSize = 32;
    for (int y = 0; y < height; y += tileSize) {
        for (int x = 0; x < width; x += tileSize) {
            int endX = std::min(x + tileSize, width);
            int endY = std::min(y + tileSize, height);
            RenderTile(x, y, endX, endY, framebuffer);
        }
    }
    
    std::vector<unsigned char> pixels(width * height * 4);
    for (int i = 0; i < width * height; ++i) {
        Vector3 color = framebuffer[i];
        
        color.x = std::sqrt(std::clamp(color.x, 0.0f, 1.0f));
        color.y = std::sqrt(std::clamp(color.y, 0.0f, 1.0f));
        color.z = std::sqrt(std::clamp(color.z, 0.0f, 1.0f));
        
        pixels[i * 4 + 0] = static_cast<unsigned char>(color.x * 255);
        pixels[i * 4 + 1] = static_cast<unsigned char>(color.y * 255);
        pixels[i * 4 + 2] = static_cast<unsigned char>(color.z * 255);
        pixels[i * 4 + 3] = 255;
    }
    
    Logger::Debug("Raytraced frame with " + std::to_string(width) + "x" + std::to_string(height) + " pixels");
    
    m_framebuffer->Unbind();
}

void RaytracingPipeline::Resize(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }
    
    m_renderData.viewportWidth = width;
    m_renderData.viewportHeight = height;
    
    if (m_initialized) {
        Cleanup();
        Initialize(width, height);
    }
    
    Logger::Info("Raytracing pipeline resized to " + std::to_string(width) + "x" + std::to_string(height));
}

void RaytracingPipeline::Shutdown() {
    Cleanup();
}

void RaytracingPipeline::BeginFrame(const RenderData& renderData) {
    m_renderData = renderData;
}

void RaytracingPipeline::EndFrame() {
    // Raytracing doesn't need special end frame handling
}

std::shared_ptr<Texture> RaytracingPipeline::GetFinalTexture() const {
    return m_colorTexture;
}

void RaytracingPipeline::Cleanup() {
    m_framebuffer.reset();
    m_colorTexture.reset();
    m_spheres.clear();
    
    m_initialized = false;
    Logger::Info("Raytracing pipeline cleaned up");
}

void RaytracingPipeline::AddSphere(const Sphere& sphere) {
    m_spheres.push_back(sphere);
}

void RaytracingPipeline::ClearScene() {
    m_spheres.clear();
}

Vector3 RaytracingPipeline::TraceRay(const Ray& ray, int depth) {
    if (depth >= m_maxBounces) {
        return Vector3(0.2f, 0.3f, 0.3f);
    }
    
    HitInfo hit = RayIntersectScene(ray);
    
    if (!hit.hit) {
        float t = 0.5f * (ray.direction.y + 1.0f);
        return Vector3(1.0f - t, 1.0f - t, 1.0f) * (1.0f - t) + Vector3(0.5f, 0.7f, 1.0f) * t;
    }
    
    Vector3 color = CalculateLighting(hit, ray.direction * -1.0f);
    
    if (hit.reflectivity > 0.0f) {
        Vector3 reflectDir = ray.direction - hit.normal * 2.0f * ray.direction.Dot(hit.normal);
        Ray reflectRay(hit.point + hit.normal * 0.001f, reflectDir);
        Vector3 reflectColor = TraceRay(reflectRay, depth + 1);
        
        color = color * (1.0f - hit.reflectivity) + reflectColor * hit.reflectivity;
    }
    
    return color;
}

HitInfo RaytracingPipeline::RayIntersectSphere(const Ray& ray, const Sphere& sphere) {
    HitInfo hit;
    
    Vector3 oc = ray.origin - sphere.center;
    float a = ray.direction.Dot(ray.direction);
    float b = 2.0f * oc.Dot(ray.direction);
    float c = oc.Dot(oc) - sphere.radius * sphere.radius;
    
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) {
        return hit;
    }
    
    float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
    if (t < 0.001f) {
        t = (-b + std::sqrt(discriminant)) / (2.0f * a);
        if (t < 0.001f) {
            return hit;
        }
    }
    
    hit.hit = true;
    hit.distance = t;
    hit.point = ray.origin + ray.direction * t;
    hit.normal = (hit.point - sphere.center) / sphere.radius;
    hit.color = sphere.color;
    hit.reflectivity = sphere.reflectivity;
    
    return hit;
}

HitInfo RaytracingPipeline::RayIntersectScene(const Ray& ray) {
    HitInfo closestHit;
    float closestDistance = std::numeric_limits<float>::max();
    
    for (const auto& sphere : m_spheres) {
        HitInfo hit = RayIntersectSphere(ray, sphere);
        if (hit.hit && hit.distance < closestDistance) {
            closestDistance = hit.distance;
            closestHit = hit;
        }
    }
    
    return closestHit;
}

Vector3 RaytracingPipeline::CalculateLighting(const HitInfo& hit, const Vector3& viewDir) {
    Vector3 lightDir = (m_lightPos - hit.point).Normalized();
    
    Ray shadowRay(hit.point + hit.normal * 0.001f, lightDir);
    HitInfo shadowHit = RayIntersectScene(shadowRay);
    
    float shadow = 1.0f;
    if (shadowHit.hit) {
        float lightDistance = (m_lightPos - hit.point).Length();
        if (shadowHit.distance < lightDistance) {
            shadow = 0.3f;
        }
    }
    
    Vector3 ambient = hit.color * 0.1f;
    
    float diff = std::max(0.0f, hit.normal.Dot(lightDir));
    Vector3 diffuse = hit.color * diff * m_lightColor * shadow;
    
    Vector3 reflectDir = lightDir - hit.normal * 2.0f * lightDir.Dot(hit.normal);
    float spec = std::pow(std::max(0.0f, viewDir.Dot(reflectDir)), 32.0f);
    Vector3 specular = m_lightColor * spec * shadow * 0.5f;
    
    return ambient + diffuse + specular;
}

Ray RaytracingPipeline::GetCameraRay(float x, float y) {
    float aspect = static_cast<float>(m_renderData.viewportWidth) / static_cast<float>(m_renderData.viewportHeight);
    float theta = m_fov * M_PI / 180.0f;
    float halfHeight = std::tan(theta / 2.0f);
    float halfWidth = aspect * halfHeight;
    
    Vector3 w = (m_cameraPos - m_cameraTarget).Normalized();
    Vector3 u = m_cameraUp.Cross(w).Normalized();
    Vector3 v = w.Cross(u);
    
    Vector3 lowerLeftCorner = m_cameraPos - halfWidth * u - halfHeight * v - w;
    Vector3 horizontal = u * 2.0f * halfWidth;
    Vector3 vertical = v * 2.0f * halfHeight;
    
    Vector3 direction = lowerLeftCorner + horizontal * x + vertical * y - m_cameraPos;
    
    return Ray(m_cameraPos, direction.Normalized());
}

void RaytracingPipeline::RenderPixel(int x, int y, std::vector<Vector3>& framebuffer) {
    Vector3 color(0.0f, 0.0f, 0.0f);
    
    for (int s = 0; s < m_samplesPerPixel; ++s) {
        float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(m_renderData.viewportWidth);
        float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(m_renderData.viewportHeight);
        
        Ray ray = GetCameraRay(u, v);
        color = color + TraceRay(ray);
    }
    
    color = color / static_cast<float>(m_samplesPerPixel);
    
    int index = y * m_renderData.viewportWidth + x;
    framebuffer[index] = color;
}

void RaytracingPipeline::RenderTile(int startX, int startY, int endX, int endY, std::vector<Vector3>& framebuffer) {
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            RenderPixel(x, y, framebuffer);
        }
    }
}

}
