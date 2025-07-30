#include "OpenGLRenderer.h"
#include "../Meshes/Mesh.h"
#include "../Shaders/Shader.h"
#include "../../Core/Logging/Logger.h"
#include "../Core/OpenGLHeaders.h"
#include <string>

namespace GameEngine {

OpenGLRenderer::OpenGLRenderer() = default;

OpenGLRenderer::~OpenGLRenderer() = default;

bool OpenGLRenderer::Initialize() {
    Logger::Info("Initializing OpenGL Renderer...");
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    const std::string vertexSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec3 aColor;
        
        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;
        
        out vec3 FragPos;
        out vec3 Normal;
        flat out vec3 VertexColor;
        
        void main() {
            FragPos = vec3(uModel * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(uModel))) * aNormal;
            VertexColor = aColor;
            
            gl_Position = uProjection * uView * vec4(FragPos, 1.0);
        }
    )";
    
    const std::string fragmentSource = R"(
        #version 330 core
        in vec3 FragPos;
        in vec3 Normal;
        flat in vec3 VertexColor;
        
        out vec4 color;
        
        uniform vec3 lightDir = vec3(-0.2, -1.0, -0.3);
        uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);
        uniform vec3 ambientColor = vec3(0.1, 0.1, 0.1);
        
        void main() {
            vec3 ambient = ambientColor * VertexColor;
            
            vec3 norm = normalize(Normal);
            vec3 lightDirection = normalize(-lightDir);
            float diff = max(dot(norm, lightDirection), 0.0);
            vec3 diffuse = diff * lightColor * VertexColor;
            
            vec3 result = ambient + diffuse;
            color = vec4(result, 1.0);
        }
    )";
    
    m_basicShader = std::make_unique<Shader>();
    if (!m_basicShader->LoadFromSource(vertexSource, fragmentSource)) {
        Logger::Error("Failed to load basic shader");
        return false;
    }
    
    Logger::Debug("Shader attribute locations - aPos: 0, aNormal: 1, aColor: 2");
    
    Logger::Info("OpenGL Renderer initialized successfully");
    return true;
}

void OpenGLRenderer::Shutdown() {
    Logger::Info("OpenGL Renderer shutdown");
}

void OpenGLRenderer::Clear(const Vector3& color) {
    glClearColor(color.x, color.y, color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void OpenGLRenderer::BeginFrame() {
}

void OpenGLRenderer::EndFrame() {
}

void OpenGLRenderer::SetProjectionMatrix(const Matrix4& projection) {
    m_projectionMatrix = projection;
}

void OpenGLRenderer::SetViewMatrix(const Matrix4& view) {
    m_viewMatrix = view;
}

void OpenGLRenderer::DrawMesh(const Mesh& mesh, const Matrix4& modelMatrix, Shader* shader) {
    Shader* activeShader = shader ? shader : m_basicShader.get();
    
    if (activeShader) {
        activeShader->Use();
        activeShader->SetMatrix4("uModel", modelMatrix);
        activeShader->SetMatrix4("uView", m_viewMatrix);
        activeShader->SetMatrix4("uProjection", m_projectionMatrix);
        
        activeShader->SetVector3("lightDir", Vector3(-0.2f, -1.0f, -0.3f));
        activeShader->SetVector3("lightColor", Vector3(1.0f, 1.0f, 1.0f));
        activeShader->SetVector3("ambientColor", Vector3(0.1f, 0.1f, 0.1f));
    }
    
    mesh.Draw();
}

void OpenGLRenderer::CreateDefaultShader() {
    Logger::Info("Using fixed function pipeline for demo");
    m_defaultShaderProgram = 0;
}

}
