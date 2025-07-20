#include "Shader.h"
#include "../../Core/Logging/Logger.h"
#include <GL/gl.h>
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

bool Shader::LoadFromSource(const std::string& /*vertexSource*/, const std::string& /*fragmentSource*/) {
    Logger::Info("Shader loading simplified for demo");
    return true;
}

void Shader::Use() const {
}

void Shader::Unuse() const {
}

void Shader::SetBool(const std::string& /*name*/, bool /*value*/) {
}

void Shader::SetInt(const std::string& /*name*/, int /*value*/) {
}

void Shader::SetFloat(const std::string& /*name*/, float /*value*/) {
}

void Shader::SetVector3(const std::string& /*name*/, const Vector3& /*value*/) {
}

void Shader::SetMatrix4(const std::string& /*name*/, const Matrix4& /*value*/) {
}

unsigned int Shader::CompileShader(const std::string& /*source*/, unsigned int /*type*/) {
    return 0;
}

bool Shader::LinkProgram(unsigned int /*vertexShader*/, unsigned int /*fragmentShader*/) {
    return true;
}

int Shader::GetUniformLocation(const std::string& /*name*/) {
    return -1;
}

unsigned int Shader::GetProgramID() const {
    return m_programID;
}

}
