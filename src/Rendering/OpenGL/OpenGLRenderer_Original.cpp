#include "OpenGLRenderer.h"
#include "../Meshes/Mesh.h"
#include "../Shaders/Shader.h"
#include "../../Core/Logging/Logger.h"
#include <GL/gl.h>
#include <string>

namespace GameEngine {

OpenGLRenderer::OpenGLRenderer() = default;

OpenGLRenderer::~OpenGLRenderer() {
    Shutdown();
}

bool OpenGLRenderer::Initialize() {
    if (m_initialized) {
        Logger::Warning("OpenGL Renderer already initialized");
        return true;
    }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    Logger::Info("OpenGL Version: " + std::string(version ? version : "Unknown"));
    
    CreateDefaultShader();
    
    m_viewMatrix = Matrix4::Identity();
    m_projectionMatrix = Matrix4::Identity();
    
    m_initialized = true;
    Logger::Info("OpenGL Renderer initialized successfully");
    return true;
}

void OpenGLRenderer::Shutdown() {
    if (m_initialized) {
        if (m_defaultShaderProgram != 0) {
            glDeleteProgram(m_defaultShaderProgram);
            m_defaultShaderProgram = 0;
        }
        
        m_initialized = false;
        Logger::Info("OpenGL Renderer shutdown");
    }
}

void OpenGLRenderer::BeginFrame() {
}

void OpenGLRenderer::EndFrame() {
}

void OpenGLRenderer::Clear(const Vector3& color) {
    glClearColor(color.x, color.y, color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void OpenGLRenderer::SetViewMatrix(const Matrix4& view) {
    m_viewMatrix = view;
}

void OpenGLRenderer::SetProjectionMatrix(const Matrix4& projection) {
    m_projectionMatrix = projection;
}

void OpenGLRenderer::DrawMesh(const Mesh& mesh, const Matrix4& modelMatrix, Shader* shader) {
    unsigned int shaderProgram = shader ? shader->GetProgramID() : m_defaultShaderProgram;
    
    if (shaderProgram == 0) {
        Logger::Error("No valid shader program for rendering");
        return;
    }
    
    glUseProgram(shaderProgram);
    
    Matrix4 mvp = m_projectionMatrix * m_viewMatrix * modelMatrix;
    
    int mvpLocation = glGetUniformLocation(shaderProgram, "u_MVP");
    if (mvpLocation != -1) {
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp.Data());
    }
    
    int modelLocation = glGetUniformLocation(shaderProgram, "u_Model");
    if (modelLocation != -1) {
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, modelMatrix.Data());
    }
    
    mesh.Bind();
    mesh.Draw();
    mesh.Unbind();
}

void OpenGLRenderer::CreateDefaultShader() {
    const char* vertexShaderSource = R"(
        #version 450 core
        
        layout (location = 0) in vec3 a_Position;
        layout (location = 1) in vec3 a_Normal;
        layout (location = 2) in vec2 a_TexCoord;
        
        uniform mat4 u_MVP;
        uniform mat4 u_Model;
        
        out vec3 v_Normal;
        out vec2 v_TexCoord;
        
        void main() {
            gl_Position = u_MVP * vec4(a_Position, 1.0);
            v_Normal = mat3(u_Model) * a_Normal;
            v_TexCoord = a_TexCoord;
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 450 core
        
        in vec3 v_Normal;
        in vec2 v_TexCoord;
        
        out vec4 FragColor;
        
        void main() {
            vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
            float diff = max(dot(normalize(v_Normal), lightDir), 0.0);
            vec3 color = vec3(0.8, 0.8, 0.8) * (0.3 + 0.7 * diff);
            FragColor = vec4(color, 1.0);
        }
    )";
    
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        Logger::Error("Vertex shader compilation failed: " + std::string(infoLog));
        return;
    }
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        Logger::Error("Fragment shader compilation failed: " + std::string(infoLog));
        glDeleteShader(vertexShader);
        return;
    }
    
    m_defaultShaderProgram = glCreateProgram();
    glAttachShader(m_defaultShaderProgram, vertexShader);
    glAttachShader(m_defaultShaderProgram, fragmentShader);
    glLinkProgram(m_defaultShaderProgram);
    
    glGetProgramiv(m_defaultShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_defaultShaderProgram, 512, nullptr, infoLog);
        Logger::Error("Shader program linking failed: " + std::string(infoLog));
        glDeleteProgram(m_defaultShaderProgram);
        m_defaultShaderProgram = 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    if (m_defaultShaderProgram != 0) {
        Logger::Info("Default shader created successfully");
    }
}

}
