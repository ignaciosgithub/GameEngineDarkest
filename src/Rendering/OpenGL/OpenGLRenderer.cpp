#include "OpenGLRenderer.h"
#include "../Meshes/Mesh.h"
#include "../Shaders/Shader.h"
#include "../../Core/Logging/Logger.h"
#include <GL/gl.h>
#include <string>

namespace GameEngine {

OpenGLRenderer::OpenGLRenderer() = default;

OpenGLRenderer::~OpenGLRenderer() = default;

bool OpenGLRenderer::Initialize() {
    Logger::Info("Initializing OpenGL Renderer (simplified for demo)...");
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    
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
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float fov = 45.0f * 3.14159f / 180.0f;
    float aspect = 1280.0f / 720.0f;
    float near = 0.1f;
    float far = 1000.0f;
    
    float f = 1.0f / tan(fov / 2.0f);
    float projection[16] = {
        f/aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (far+near)/(near-far), (2*far*near)/(near-far),
        0, 0, -1, 0
    };
    glLoadMatrixf(projection);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glTranslatef(0.0f, -5.0f, -10.0f);
}

void OpenGLRenderer::EndFrame() {
}

void OpenGLRenderer::SetProjectionMatrix(const Matrix4& projection) {
    m_projectionMatrix = projection;
}

void OpenGLRenderer::SetViewMatrix(const Matrix4& view) {
    m_viewMatrix = view;
}

void OpenGLRenderer::DrawMesh(const Mesh& /*mesh*/, const Matrix4& modelMatrix, Shader* /*shader*/) {
    glPushMatrix();
    
    const float* data = modelMatrix.Data();
    glMultMatrixf(data);
    
    glBegin(GL_QUADS);
    
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    
    glEnd();
    glPopMatrix();
}

void OpenGLRenderer::CreateDefaultShader() {
    Logger::Info("Using fixed function pipeline for demo");
    m_defaultShaderProgram = 0;
}

}
