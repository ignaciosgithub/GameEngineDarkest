#pragma once

#include "../Renderer.h"
#include "../../Core/Math/Matrix4.h"
#include <memory>

namespace GameEngine {
    class Shader;
    
    class OpenGLRenderer : public Renderer {
    public:
        OpenGLRenderer();
        ~OpenGLRenderer() override;
        
        bool Initialize() override;
        void Shutdown() override;
        
        void BeginFrame() override;
        void EndFrame() override;
        
        void Clear(const Vector3& color = Vector3(0.0f, 0.0f, 0.0f)) override;
        void SetViewport(int x, int y, int width, int height) override;
        
        void SetViewMatrix(const Matrix4& view) override;
        void SetProjectionMatrix(const Matrix4& projection) override;
        
        void DrawMesh(const Mesh& mesh, const Matrix4& modelMatrix, Shader* shader = nullptr) override;
        
    private:
        Matrix4 m_viewMatrix;
        Matrix4 m_projectionMatrix;
        
        std::unique_ptr<Shader> m_basicShader;
        unsigned int m_defaultShaderProgram = 0;
        bool m_initialized = false;
        
        void CreateDefaultShader();
    };
}
