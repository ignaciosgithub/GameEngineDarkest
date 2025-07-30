#include "RaytracingPipeline.h"
#include "../../Core/Logging/Logger.h"
#include "../../Core/ECS/World.h"
#include "../Core/OpenGLHeaders.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    
    AddTriangle(Triangle(Vector3(-1, -1, -5), Vector3(1, -1, -5), Vector3(0, 1, -5), Vector3(0.8f, 0.3f, 0.3f), 0.1f));
    AddTriangle(Triangle(Vector3(-3, -1, -8), Vector3(-1, -1, -8), Vector3(-2, 1, -8), Vector3(0.3f, 0.8f, 0.3f), 0.3f));
    AddTriangle(Triangle(Vector3(1, -1, -6), Vector3(3, -1, -6), Vector3(2, 1, -6), Vector3(0.3f, 0.3f, 0.8f), 0.5f));
    
    if (m_useComputeShader) {
        m_computeShader = std::make_shared<Shader>();
        
        std::string computeShaderPath = "src/Rendering/Shaders/raytracing.comp";
        if (!m_computeShader->LoadComputeShader(computeShaderPath)) {
            Logger::Warning("Failed to load compute shader, falling back to CPU raytracing");
            m_useComputeShader = false;
        } else {
            SetupComputeShaderBuffers();
        }
    }
    
    BuildBVH();
    
    m_initialized = true;
    Logger::Info("Raytracing pipeline initialized with " + std::to_string(m_spheres.size()) + " spheres and " + std::to_string(m_triangles.size()) + " triangles");
    return true;
}

void RaytracingPipeline::Render(World* /*world*/) {
    if (!m_initialized) {
        return;
    }
    
    m_framebuffer->Bind();
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
    if (m_triangleSSBO != 0) {
        Logger::Debug("Cleaning up triangle SSBO (simplified)");
        m_triangleSSBO = 0;
    }
    
    if (m_bvhSSBO != 0) {
        Logger::Debug("Cleaning up BVH SSBO (simplified)");
        m_bvhSSBO = 0;
    }
    
    m_computeShader.reset();
    m_framebuffer.reset();
    m_colorTexture.reset();
    m_spheres.clear();
    m_triangles.clear();
    m_bvhNodes.clear();
    
    m_initialized = false;
    Logger::Info("Raytracing pipeline cleaned up");
}

void RaytracingPipeline::AddSphere(const Sphere& sphere) {
    m_spheres.push_back(sphere);
    Logger::Info("Added sphere to raytracing scene. Total spheres: " + std::to_string(m_spheres.size()));
}

void RaytracingPipeline::AddTriangle(const Triangle& triangle) {
    m_triangles.push_back(triangle);
    Logger::Info("Added triangle to raytracing scene. Total triangles: " + std::to_string(m_triangles.size()));
}

void RaytracingPipeline::ClearScene() {
    m_spheres.clear();
    m_triangles.clear();
    m_bvhNodes.clear();
    Logger::Info("Cleared raytracing scene");
}

void RaytracingPipeline::BuildBVH() {
    if (m_triangles.empty()) {
        Logger::Warning("Cannot build BVH: no triangles in scene");
        return;
    }
    
    m_bvhNodes.clear();
    m_bvhNodes.resize(m_triangles.size() * 2);
    
    std::vector<int> triangleIndices(m_triangles.size());
    for (size_t i = 0; i < m_triangles.size(); ++i) {
        triangleIndices[i] = static_cast<int>(i);
    }
    
    BVHNode& root = m_bvhNodes[0];
    root.triangleStart = 0;
    root.triangleCount = static_cast<int>(m_triangles.size());
    
    Vector3 minBounds(1e30f, 1e30f, 1e30f);
    Vector3 maxBounds(-1e30f, -1e30f, -1e30f);
    
    for (const Triangle& tri : m_triangles) {
        minBounds.x = std::min({minBounds.x, tri.v0.x, tri.v1.x, tri.v2.x});
        minBounds.y = std::min({minBounds.y, tri.v0.y, tri.v1.y, tri.v2.y});
        minBounds.z = std::min({minBounds.z, tri.v0.z, tri.v1.z, tri.v2.z});
        
        maxBounds.x = std::max({maxBounds.x, tri.v0.x, tri.v1.x, tri.v2.x});
        maxBounds.y = std::max({maxBounds.y, tri.v0.y, tri.v1.y, tri.v2.y});
        maxBounds.z = std::max({maxBounds.z, tri.v0.z, tri.v1.z, tri.v2.z});
    }
    
    root.minBounds = minBounds;
    root.maxBounds = maxBounds;
    
    BuildBVHRecursive(0, triangleIndices);
    
    Logger::Info("Built BVH with " + std::to_string(m_bvhNodes.size()) + " nodes for " + std::to_string(m_triangles.size()) + " triangles");
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
    Vector3 lightDirection = m_lightPos - hit.point;
    Vector3 lightDir;
    if (lightDirection.LengthSquared() > 0.0001f) {
        lightDir = lightDirection.Normalized();
    } else {
        lightDir = Vector3(0.0f, 1.0f, 0.0f); // Default upward direction
    }
    
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
    float theta = m_fov * static_cast<float>(M_PI) / 180.0f;
    float halfHeight = std::tan(theta / 2.0f);
    float halfWidth = aspect * halfHeight;
    
    Vector3 wDirection = m_cameraPos - m_cameraTarget;
    Vector3 w;
    if (wDirection.LengthSquared() > 0.0001f) {
        w = wDirection.Normalized();
    } else {
        w = Vector3(0.0f, 0.0f, 1.0f); // Default forward direction
    }
    
    Vector3 uCross = m_cameraUp.Cross(w);
    Vector3 u;
    if (uCross.LengthSquared() > 0.0001f) {
        u = uCross.Normalized();
    } else {
        u = Vector3(1.0f, 0.0f, 0.0f); // Default right direction
    }
    Vector3 v = w.Cross(u);
    
    Vector3 lowerLeftCorner = m_cameraPos - halfWidth * u - halfHeight * v - w;
    Vector3 horizontal = u * 2.0f * halfWidth;
    Vector3 vertical = v * 2.0f * halfHeight;
    
    Vector3 direction = lowerLeftCorner + horizontal * x + vertical * y - m_cameraPos;
    
    if (direction.LengthSquared() > 0.0001f) {
        direction = direction.Normalized();
    } else {
        direction = Vector3(0.0f, 0.0f, -1.0f); // Default forward direction
    }
    
    return Ray(m_cameraPos, direction);
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

HitInfo RaytracingPipeline::RayIntersectTriangle(const Ray& ray, const Triangle& triangle) {
    HitInfo hit;
    
    Vector3 edge1 = triangle.v1 - triangle.v0;
    Vector3 edge2 = triangle.v2 - triangle.v0;
    Vector3 h = ray.direction.Cross(edge2);
    float a = edge1.Dot(h);
    
    if (a > -0.00001f && a < 0.00001f) {
        return hit; // Ray is parallel to triangle
    }
    
    float f = 1.0f / a;
    Vector3 s = ray.origin - triangle.v0;
    float u = f * s.Dot(h);
    
    if (u < 0.0f || u > 1.0f) {
        return hit;
    }
    
    Vector3 q = s.Cross(edge1);
    float v = f * ray.direction.Dot(q);
    
    if (v < 0.0f || u + v > 1.0f) {
        return hit;
    }
    
    float t = f * edge2.Dot(q);
    
    if (t > 0.00001f) {
        hit.hit = true;
        hit.distance = t;
        hit.point = ray.origin + ray.direction * t;
        hit.normal = triangle.normal;
        hit.color = triangle.color;
        hit.reflectivity = triangle.reflectivity;
    }
    
    return hit;
}

void RaytracingPipeline::BuildBVHRecursive(int nodeIndex, std::vector<int>& triangleIndices) {
    BVHNode& node = m_bvhNodes[nodeIndex];
    
    if (triangleIndices.size() <= 4) {
        node.triangleStart = static_cast<int>(triangleIndices.size() > 0 ? triangleIndices[0] : 0);
        node.triangleCount = static_cast<int>(triangleIndices.size());
        return;
    }
    
    Vector3 extent = node.maxBounds - node.minBounds;
    int splitAxis = 0;
    if (extent.y > extent.x) splitAxis = 1;
    if (extent.z > extent[splitAxis]) splitAxis = 2;
    
    std::sort(triangleIndices.begin(), triangleIndices.end(), 
        [this, splitAxis](int a, int b) {
            Vector3 centroidA = (m_triangles[a].v0 + m_triangles[a].v1 + m_triangles[a].v2) / 3.0f;
            Vector3 centroidB = (m_triangles[b].v0 + m_triangles[b].v1 + m_triangles[b].v2) / 3.0f;
            return centroidA[splitAxis] < centroidB[splitAxis];
        });
    
    size_t mid = triangleIndices.size() / 2;
    std::vector<int> leftTriangles(triangleIndices.begin(), triangleIndices.begin() + mid);
    std::vector<int> rightTriangles(triangleIndices.begin() + mid, triangleIndices.end());
    
    int leftChildIndex = static_cast<int>(m_bvhNodes.size());
    int rightChildIndex = leftChildIndex + 1;
    
    node.leftChild = leftChildIndex;
    node.rightChild = rightChildIndex;
    
    m_bvhNodes.resize(m_bvhNodes.size() + 2);
    
    BVHNode& leftChild = m_bvhNodes[leftChildIndex];
    BVHNode& rightChild = m_bvhNodes[rightChildIndex];
    
    if (!leftTriangles.empty()) {
        leftChild.minBounds = Vector3(std::numeric_limits<float>::max());
        leftChild.maxBounds = Vector3(std::numeric_limits<float>::lowest());
        
        for (int triIndex : leftTriangles) {
            const Triangle& tri = m_triangles[triIndex];
            leftChild.minBounds = Vector3::Min(leftChild.minBounds, Vector3::Min(Vector3::Min(tri.v0, tri.v1), tri.v2));
            leftChild.maxBounds = Vector3::Max(leftChild.maxBounds, Vector3::Max(Vector3::Max(tri.v0, tri.v1), tri.v2));
        }
    }
    
    if (!rightTriangles.empty()) {
        rightChild.minBounds = Vector3(std::numeric_limits<float>::max());
        rightChild.maxBounds = Vector3(std::numeric_limits<float>::lowest());
        
        for (int triIndex : rightTriangles) {
            const Triangle& tri = m_triangles[triIndex];
            rightChild.minBounds = Vector3::Min(rightChild.minBounds, Vector3::Min(Vector3::Min(tri.v0, tri.v1), tri.v2));
            rightChild.maxBounds = Vector3::Max(rightChild.maxBounds, Vector3::Max(Vector3::Max(tri.v0, tri.v1), tri.v2));
        }
    }
    
    BuildBVHRecursive(leftChildIndex, leftTriangles);
    BuildBVHRecursive(rightChildIndex, rightTriangles);
}

bool RaytracingPipeline::RayBoxIntersect(const Ray& ray, const Vector3& boxMin, const Vector3& boxMax) {
    Vector3 invDir = Vector3(1.0f / ray.direction.x, 1.0f / ray.direction.y, 1.0f / ray.direction.z);
    Vector3 t1 = (boxMin - ray.origin) * invDir;
    Vector3 t2 = (boxMax - ray.origin) * invDir;
    
    Vector3 tMin = Vector3::Min(t1, t2);
    Vector3 tMax = Vector3::Max(t1, t2);
    
    float tNear = std::max(std::max(tMin.x, tMin.y), tMin.z);
    float tFar = std::min(std::min(tMax.x, tMax.y), tMax.z);
    
    return tNear <= tFar && tFar > 0.0f;
}

HitInfo RaytracingPipeline::TraverseBVH(const Ray& ray) {
    HitInfo closestHit;
    float closestDistance = std::numeric_limits<float>::max();
    
    if (m_bvhNodes.empty()) {
        return closestHit;
    }
    
    std::vector<int> nodeStack;
    nodeStack.reserve(64);
    nodeStack.push_back(0);
    
    while (!nodeStack.empty()) {
        int nodeIndex = nodeStack.back();
        nodeStack.pop_back();
        
        const BVHNode& node = m_bvhNodes[nodeIndex];
        
        if (!RayBoxIntersect(ray, node.minBounds, node.maxBounds)) {
            continue;
        }
        
        if (node.triangleCount > 0) {
            for (int i = 0; i < node.triangleCount; ++i) {
                int triIndex = node.triangleStart + i;
                if (triIndex < static_cast<int>(m_triangles.size())) {
                    HitInfo hit = RayIntersectTriangle(ray, m_triangles[triIndex]);
                    if (hit.hit && hit.distance < closestDistance) {
                        closestDistance = hit.distance;
                        closestHit = hit;
                    }
                }
            }
        } else {
            if (node.leftChild >= 0) {
                nodeStack.push_back(node.leftChild);
            }
            if (node.rightChild >= 0) {
                nodeStack.push_back(node.rightChild);
            }
        }
    }
    
    return closestHit;
}

void RaytracingPipeline::RenderWithComputeShader() {
    if (!m_computeShader || !m_useComputeShader) {
        Logger::Warning("Compute shader not available, falling back to CPU raytracing");
        return;
    }
    
    Logger::Info("Rendering with compute shader (simplified implementation)");
    
    
    m_computeShader->Use();
    
    m_computeShader->SetVector3("cameraPos", m_cameraPos);
    m_computeShader->SetVector3("cameraTarget", m_cameraTarget);
    m_computeShader->SetVector3("cameraUp", m_cameraUp);
    m_computeShader->SetFloat("fov", m_fov);
    m_computeShader->SetVector3("lightPos", m_lightPos);
    m_computeShader->SetVector3("lightColor", m_lightColor);
    m_computeShader->SetInt("maxBounces", m_maxBounces);
}

void RaytracingPipeline::SetupComputeShaderBuffers() {
    Logger::Info("Setting up compute shader buffers (simplified implementation)");
    
    
    Logger::Info("Triangle SSBO setup (simplified): " + std::to_string(m_triangles.size()) + " triangles");
    Logger::Info("BVH SSBO setup (simplified): " + std::to_string(m_bvhNodes.size()) + " nodes");
    
    m_triangleSSBO = 1;
    m_bvhSSBO = 2;
}

}
