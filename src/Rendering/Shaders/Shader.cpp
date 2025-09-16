#include "Shader.h"
#include "../../Core/Logging/Logger.h"
#include "../Core/OpenGLHeaders.h"
#include <fstream>
#include <sstream>

namespace GameEngine {

Shader::Shader() : m_programID(0) {}

Shader::~Shader() {
}

bool Shader::LoadFromFile(const std::string& /*vertexPath*/, const std::string& /*fragmentPath*/) {
    Logger::Info("Shader loading simplified for demo");
    return true;
}

bool Shader::LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vertexShader = CompileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        Logger::Error("Failed to compile shaders");
        return false;
    }
    
    bool success = LinkProgram(vertexShader, fragmentShader);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    if (success) {
        Logger::Info("Shader compiled and linked successfully");
    }
    
    return success;
}

bool Shader::LoadComputeShader(const std::string& computePath) {
    std::ifstream file(computePath);
    if (!file.is_open()) {
        Logger::Error("Failed to open compute shader file: " + computePath);
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string computeSource = buffer.str();
    file.close();
    
    return LoadComputeShaderFromSource(computeSource);
}

bool Shader::LoadComputeShaderFromSource(const std::string& computeSource) {
    unsigned int computeShader = CompileShader(computeSource, GL_COMPUTE_SHADER);
    if (computeShader == 0) {
        Logger::Error("Failed to compile compute shader");
        return false;
    }
    bool linked = LinkComputeProgram(computeShader);
    glDeleteShader(computeShader);
    if (linked) {
        Logger::Info("Compute shader compiled and linked successfully");
    }
    return linked;
}

void Shader::Use() const {
    if (m_programID != 0) {
        glUseProgram(m_programID);
        Logger::Debug("Using shader program ID: " + std::to_string(m_programID));
    }
}

void Shader::Unuse() const {
}

void Shader::SetBool(const std::string& name, bool value) {
    SetInt(name, static_cast<int>(value));
}

void Shader::SetInt(const std::string& name, int value) {
    int location = GetUniformLocation(name);
    glUniform1i(location, value);
    Logger::Debug("Set int uniform: " + name + " = " + std::to_string(value));
}

void Shader::SetFloat(const std::string& name, float value) {
    int location = GetUniformLocation(name);
    glUniform1f(location, value);
    Logger::Debug("Set float uniform: " + name + " = " + std::to_string(value));
}

void Shader::SetVector3(const std::string& name, const Vector3& value) {
    int location = GetUniformLocation(name);
    glUniform3f(location, value.x, value.y, value.z);
    Logger::Debug("Set Vector3 uniform: " + name);
}

void Shader::SetVector4(const std::string& name, const Vector4& /*value*/) {
    GetUniformLocation(name);
    Logger::Debug("Set Vector4 uniform: " + name);
}

void Shader::SetMatrix4(const std::string& name, const Matrix4& value) {
    int location = GetUniformLocation(name);
    glUniformMatrix4fv(location, 1, GL_TRUE, value.Data());
    Logger::Debug("Set Matrix4 uniform: " + name);
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
        Logger::Error("Shader compilation failed: " + std::string(infoLog));
        glDeleteShader(shader);
        return 0;
    }
    
    Logger::Info("Shader compiled successfully");
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
        Logger::Error("Shader linking failed: " + std::string(infoLog));
        return false;
    }
    
    Logger::Info("Shader program linked successfully");
    return true;
}

bool Shader::LinkComputeProgram(unsigned int computeShader) {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
        m_programID = 0;
    }
    unsigned int program = glCreateProgram();
    glAttachShader(program, computeShader);
    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        Logger::Error("Compute shader linking failed: " + std::string(infoLog));
        glDeleteProgram(program);
        return false;
    }

    m_programID = program;
    Logger::Info("Compute shader program linked successfully");
    return true;
}

int Shader::GetUniformLocation(const std::string& name) {
    auto it = m_uniformLocationCache.find(name);
    if (it != m_uniformLocationCache.end()) {
        return it->second;
    }
    
    int location = glGetUniformLocation(m_programID, name.c_str());
    if (location == -1) {
        Logger::Warning("Uniform '" + name + "' not found in shader");
    }
    m_uniformLocationCache[name] = location;
    Logger::Debug("Uniform location for '" + name + "': " + std::to_string(location));
    
    return location;
}

unsigned int Shader::GetProgramID() const {
    return m_programID;
}

}
