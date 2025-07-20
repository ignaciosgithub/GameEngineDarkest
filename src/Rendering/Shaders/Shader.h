#pragma once

#include <string>
#include <unordered_map>
#include "../../Core/Math/Matrix4.h"
#include "../../Core/Math/Vector3.h"

namespace GameEngine {
    class Shader {
    public:
        Shader();
        ~Shader();
        
        bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
        bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
        
        void Use() const;
        void Unuse() const;
        
        unsigned int GetProgramID() const;
        
        // Uniform setters
        void SetBool(const std::string& name, bool value);
        void SetInt(const std::string& name, int value);
        void SetFloat(const std::string& name, float value);
        void SetVector3(const std::string& name, const Vector3& value);
        void SetMatrix4(const std::string& name, const Matrix4& value);
        
    private:
        unsigned int CompileShader(const std::string& source, unsigned int type);
        bool LinkProgram(unsigned int vertexShader, unsigned int fragmentShader);
        int GetUniformLocation(const std::string& name);
        
        unsigned int m_programID = 0;
        mutable std::unordered_map<std::string, int> m_uniformLocationCache;
    };
}
