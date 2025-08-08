#include "OBJLoader.h"
#include "../../Core/Logging/Logger.h"
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <filesystem>

namespace GameEngine {
    Mesh OBJLoader::LoadFromFile(const std::string& filepath) {
        Logger::Info("Loading OBJ file: " + filepath);
        
        if (!std::filesystem::exists(filepath)) {
            Logger::Error("OBJ file does not exist: " + filepath);
            Logger::Error("Current working directory: " + std::filesystem::current_path().string());
            return Mesh();
        }
        
        try {
            auto fileSize = std::filesystem::file_size(filepath);
            Logger::Debug("OBJ file size: " + std::to_string(fileSize) + " bytes");
        } catch (const std::exception& e) {
            Logger::Warning("Could not get file size for: " + filepath + " - " + e.what());
        }
        
        OBJData data;
        if (!ParseOBJFile(filepath, data)) {
            Logger::Error("Failed to parse OBJ file: " + filepath);
            return Mesh();
        }
        
        if (data.normals.empty()) {
            ComputeMissingNormals(data);
        }
        
        if (data.vertices.empty()) {
            Logger::Warning("OBJ file contains no vertices: " + filepath);
            return Mesh();
        }
        
        Logger::Info("Successfully loaded OBJ file with " + std::to_string(data.vertices.size()) + 
                    " vertices and " + std::to_string(data.indices.size()) + " indices");
        
        return CreateMeshFromOBJData(data);
    }
    
    bool OBJLoader::ParseOBJFile(const std::string& filepath, OBJData& data) {
        std::ifstream file(filepath, std::ios::in);
        if (!file.is_open()) {
            Logger::Error("Cannot open OBJ file: " + filepath);
            Logger::Error("Current working directory or file permissions may be incorrect");
            return false;
        }
        
        std::string line;
        int lineNumber = 0;
        
        while (std::getline(file, line)) {
            lineNumber++;
            line = TrimString(line);
            
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            try {
                if (StartsWith(line, "v ")) {
                    Vector3 position = ParseVector3(line.substr(2));
                    data.positions.push_back(position);
                }
                else if (StartsWith(line, "vn ")) {
                    Vector3 normal = ParseVector3(line.substr(3));
                    data.normals.push_back(normal);
                }
                else if (StartsWith(line, "vt ")) {
                    std::vector<std::string> comps = SplitString(TrimString(line.substr(3)), ' ');
                    float u = 0.0f, v = 0.0f, w = 0.0f;
                    if (comps.size() >= 1 && IsValidFloat(comps[0])) u = std::stof(comps[0]);
                    if (comps.size() >= 2 && IsValidFloat(comps[1])) v = std::stof(comps[1]);
                    if (comps.size() >= 3 && IsValidFloat(comps[2])) w = std::stof(comps[2]);
                    data.texCoords.push_back(Vector3(u, v, w));
                }
                else if (StartsWith(line, "f ")) {
                    ParseFace(line.substr(2), data);
                }
                else if (StartsWith(line, "o ") || StartsWith(line, "g ")) {
                    data.currentGroup = TrimString(line.substr(2));
                }
                else if (StartsWith(line, "mtllib ")) {
                    std::string lib = TrimString(line.substr(7));
                    data.materialLibs[lib] = lib;
                }
                else if (StartsWith(line, "usemtl ")) {
                    data.currentMaterial = TrimString(line.substr(7));
                }
                else if (StartsWith(line, "s ")) {
                    std::string s = TrimString(line.substr(2));
                    data.smoothing = (s != "off" && s != "0");
                }
            }
            catch (const std::exception& e) {
                Logger::Warning("Error parsing line " + std::to_string(lineNumber) + 
                               " in OBJ file " + filepath + ": " + e.what());
            }
        }
        
        file.close();
        
        Logger::Debug("Parsed OBJ file: " + std::to_string(data.positions.size()) + " positions, " +
                     std::to_string(data.normals.size()) + " normals, " +
                     std::to_string(data.texCoords.size()) + " texture coordinates, " +
                     std::to_string(data.vertices.size()) + " vertices");
        
        return !data.positions.empty();
    }
    
    Vector3 OBJLoader::ParseVector3(const std::string& line) {
        std::vector<std::string> components = SplitString(TrimString(line), ' ');
        
        if (components.size() < 1) {
            throw std::runtime_error("Invalid vector format");
        }
        
        float x = 0.0f, y = 0.0f, z = 0.0f;
        
        if (components.size() >= 1 && IsValidFloat(components[0])) {
            x = std::stof(components[0]);
        }
        if (components.size() >= 2 && IsValidFloat(components[1])) {
            y = std::stof(components[1]);
        }
        if (components.size() >= 3 && IsValidFloat(components[2])) {
            z = std::stof(components[2]);
        }
        
        return Vector3(x, y, z);
    }
    
    void OBJLoader::ParseFace(const std::string& line, OBJData& data) {
        std::vector<std::string> faceVertices = SplitString(TrimString(line), ' ');
        
        if (faceVertices.size() < 3) {
            throw std::runtime_error("Face must have at least 3 vertices");
        }
        
        std::vector<Vertex> faceVertexData;
        faceVertexData.reserve(faceVertices.size());
        
        for (const std::string& vertexStr : faceVertices) {
            std::vector<std::string> indices = SplitString(vertexStr, '/');
            if (indices.empty()) continue;
            
            Vertex vertex;
            vertex.position = Vector3::Zero;
            vertex.normal = Vector3(0.0f, 1.0f, 0.0f);
            vertex.color = Vector3(1.0f, 1.0f, 1.0f);
            vertex.texCoords = Vector3::Zero;
            
            if (!indices[0].empty()) {
                int posIndex = std::stoi(indices[0]);
                posIndex = ResolveIndex(posIndex, static_cast<int>(data.positions.size()));
                if (posIndex >= 0 && posIndex < static_cast<int>(data.positions.size())) {
                    vertex.position = data.positions[posIndex];
                }
            }
            
            if (indices.size() > 1 && !indices[1].empty()) {
                int texIndex = std::stoi(indices[1]);
                texIndex = ResolveIndex(texIndex, static_cast<int>(data.texCoords.size()));
                if (texIndex >= 0 && texIndex < static_cast<int>(data.texCoords.size())) {
                    vertex.texCoords = data.texCoords[texIndex];
                }
            }
            
            if (indices.size() > 2 && !indices[2].empty()) {
                int normalIndex = std::stoi(indices[2]);
                normalIndex = ResolveIndex(normalIndex, static_cast<int>(data.normals.size()));
                if (normalIndex >= 0 && normalIndex < static_cast<int>(data.normals.size())) {
                    vertex.normal = data.normals[normalIndex];
                }
            }
            
            faceVertexData.push_back(vertex);
        }
        
        for (size_t i = 1; i < faceVertexData.size() - 1; ++i) {
            data.vertices.push_back(faceVertexData[0]);
            data.vertices.push_back(faceVertexData[i]);
            data.vertices.push_back(faceVertexData[i + 1]);
            
            unsigned int baseIndex = static_cast<unsigned int>(data.vertices.size() - 3);
            data.indices.push_back(baseIndex);
            data.indices.push_back(baseIndex + 1);
            data.indices.push_back(baseIndex + 2);
        }
    }
    
    Mesh OBJLoader::CreateMeshFromOBJData(const OBJData& data) {
        Mesh mesh;
        
        if (!data.vertices.empty()) {
            mesh.SetVertices(data.vertices);
        }
        
        if (!data.indices.empty()) {
            mesh.SetIndices(data.indices);
        }
        
        mesh.Upload();
        
        Logger::Debug("Created mesh from OBJ data with " + std::to_string(data.vertices.size()) + 
                     " vertices and " + std::to_string(data.indices.size()) + " indices");
        
        return mesh;
    }
    
    std::vector<std::string> OBJLoader::SplitString(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        
        while (std::getline(ss, token, delimiter)) {
            token = TrimString(token);
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }
        
        return tokens;
    }
    
    std::string OBJLoader::TrimString(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            return "";
        }
        
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
    
    bool OBJLoader::IsValidFloat(const std::string& str) {
        if (str.empty()) return false;
        try {
            (void)std::stof(str);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    bool OBJLoader::StartsWith(const std::string& s, const char* prefix) {
        size_t n = std::char_traits<char>::length(prefix);
        return s.size() >= n && std::equal(prefix, prefix + n, s.begin());
    }
    
    int OBJLoader::ResolveIndex(int idx, int size) {
        if (idx > 0) return idx - 1;
        if (idx < 0) return size + idx;
        return -1;
    }
    
    void OBJLoader::ComputeMissingNormals(OBJData& data) {
        if (!data.vertices.empty()) {
            for (size_t i = 0; i + 2 < data.vertices.size(); i += 3) {
                Vector3 a = data.vertices[i + 0].position;
                Vector3 b = data.vertices[i + 1].position;
                Vector3 c = data.vertices[i + 2].position;
                Vector3 n = (b - a).Cross(c - a).Normalized();
                data.vertices[i + 0].normal = n;
                data.vertices[i + 1].normal = n;
                data.vertices[i + 2].normal = n;
            }
        }
    }
}
