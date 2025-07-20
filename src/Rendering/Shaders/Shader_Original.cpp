#include "Shader.h"
#include "../../Core/Logging/Logger.h"
#include "../Core/OpenGLHeaders.h"
#include <fstream>
#include <sstream>

namespace GameEngine {

Shader::Shader() = default;

Shader::~Shader() {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
    }
}

bool Shader::LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource, fragmentSource;
    
    std::ifstream vertexFile(vertexPath);
    if (!vertexFile.is_open()) {
        Logger::Error("Failed to open vertex shader file: " + vertexPath);
        return false;
    }
    
    std::stringstream vertexStream;
    vertexStream << vertexFile.rdbuf();
    vertexSource = vertexStream.str();
    vertexFile.close();
    
    std::ifstream fragmentFile(fragmentPath);
    if (!fragmentFile.is_open()) {
        Logger::Error("Failed to open fragment shader file: " + fragmentPath);
        return false;
    }
    
    std::stringstream fragmentStream;
    fragmentStream << fragmentFile.rdbuf();
    fragmentSource = fragmentStream.str();
    fragmentFile.close();
    
    return LoadFromSource(vertexSource, fragmentSource);
}

bool Shader::LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
        m_programID = 0;
        m_uniformLocationCache.clear();
    }
    
    unsigned int vertexShader = CompileShader(vertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        Logger::Error("Failed to compile vertex shader");
        return false;
    }
    
    unsigned int fragmentShader = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        Logger::Error("Failed to compile fragment shader");
        glDeleteShader(vertexShader);
        return false;
    }
    
    bool success = LinkProgram(vertexShader, fragmentShader);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    if (success) {
        Logger::Info("Shader program created successfully");
    }
    
    return success;
}

void Shader::Use() const {
    if (m_programID != 0) {
        glUseProgram(m_programID);
    }
}

void Shader::Unuse() const {
    glUseProgram(0);
}

void Shader::SetBool(const std::string& name, bool value) {
    glUniform1i(GetUniformLocation(name), static_cast<int>(value));
}

void Shader::SetInt(const std::string& name, int value) {
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value) {
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVector3(const std::string& name, const Vector3& value) {
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void Shader::SetMatrix4(const std::string& name, const Matrix4& value) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, value.Data());
}

unsigned int Shader::CompileShader(const std::string& source, unsigned int type) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        
        std::string shaderType = (type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment";
        Logger::Error(shaderType + " shader compilation failed: " + std::string(infoLog));
        
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool Shader::LinkProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertexShader);
    glAttachShader(m_programID, fragmentShader);
    glLinkProgram(m_programID);
    
    int success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programID, 512, nullptr, infoLog);
        Logger::Error("Shader program linking failed: " + std::string(infoLog));
        
        glDeleteProgram(m_programID);
        m_programID = 0;
        return false;
    }
    
    return true;
}

int Shader::GetUniformLocation(const std::string& name) {
    auto it = m_uniformLocationCache.find(name);
    if (it != m_uniformLocationCache.end()) {
        return it->second;
    }
    
    int location = glGetUniformLocation(m_programID, name.c_str());
    m_uniformLocationCache[name] = location;
    
    if (location == -1) {
        Logger::Warning("Uniform '" + name + "' not found in shader");
    }
    
    return location;
}

}
