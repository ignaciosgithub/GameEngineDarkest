#pragma once

#include "../Core/Math/Matrix4.h"
#include "../Core/Math/Vector3.h"
#include <memory>

namespace GameEngine {
    class Mesh;
    class Shader;
    
    enum class RendererAPI {
        None = 0,
        OpenGL = 1,
        DirectX11 = 2,
        DirectX12 = 3,
        Vulkan = 4
    };
    
    class Renderer {
    public:
        virtual ~Renderer() = default;
        
        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        
        virtual void Clear(const Vector3& color = Vector3(0.0f, 0.0f, 0.0f)) = 0;
        virtual void SetViewport(int x, int y, int width, int height) = 0;
        
        virtual void SetViewMatrix(const Matrix4& view) = 0;
        virtual void SetProjectionMatrix(const Matrix4& projection) = 0;
        
        virtual void DrawMesh(const Mesh& mesh, const Matrix4& modelMatrix, Shader* shader = nullptr) = 0;
        
        static std::unique_ptr<Renderer> Create(RendererAPI api = RendererAPI::OpenGL);
        static RendererAPI GetAPI() { return s_api; }
        
    private:
        static RendererAPI s_api;
    };
}
