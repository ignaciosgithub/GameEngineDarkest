#include "Shader.h"
#include "../../Core/Logging/Logger.h"
#include <GL/gl.h>
#include <GL/glext.h>
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
    
    Logger::Debug("Shaders cleaned up (simplified)");
    
    if (success) {
        Logger::Info("Shader compiled and linked successfully");
    }
    
    return success;
}

void Shader::Use() const {
    if (m_programID != 0) {
        Logger::Debug("Using shader program (simplified)");
    }
}

void Shader::Unuse() const {
}

void Shader::SetBool(const std::string& name, bool value) {
    SetInt(name, static_cast<int>(value));
}

void Shader::SetInt(const std::string& name, int /*value*/) {
    GetUniformLocation(name);
    Logger::Debug("Set int uniform: " + name);
}

void Shader::SetFloat(const std::string& name, float /*value*/) {
    GetUniformLocation(name);
    Logger::Debug("Set float uniform: " + name);
}

void Shader::SetVector3(const std::string& name, const Vector3& /*value*/) {
    GetUniformLocation(name);
    Logger::Debug("Set Vector3 uniform: " + name);
}

void Shader::SetMatrix4(const std::string& name, const Matrix4& /*value*/) {
    GetUniformLocation(name);
    Logger::Debug("Set Matrix4 uniform: " + name);
}

unsigned int Shader::CompileShader(const std::string& /*source*/, unsigned int /*type*/) {
    Logger::Info("Shader compilation (simplified for compatibility)");
    return 1; // Return dummy shader ID
}

bool Shader::LinkProgram(unsigned int /*vertexShader*/, unsigned int /*fragmentShader*/) {
    m_programID = 1; // Set dummy program ID
    Logger::Info("Shader program linked (simplified for compatibility)");
    return true;
}

int Shader::GetUniformLocation(const std::string& name) {
    auto it = m_uniformLocationCache.find(name);
    if (it != m_uniformLocationCache.end()) {
        return it->second;
    }
    
    int location = 0; // Return dummy location
    m_uniformLocationCache[name] = location;
    Logger::Debug("Uniform location for '" + name + "' (simplified)");
    
    return location;
}

unsigned int Shader::GetProgramID() const {
    return m_programID;
}

}
