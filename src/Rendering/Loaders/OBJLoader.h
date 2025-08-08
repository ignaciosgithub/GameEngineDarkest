#pragma once

#include "../Meshes/Mesh.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace GameEngine {
    class OBJLoader {
    public:
        static Mesh LoadFromFile(const std::string& filepath);
        
    private:
        struct OBJData {
            std::vector<Vector3> positions;
            std::vector<Vector3> normals;
            std::vector<Vector3> texCoords;
            std::vector<unsigned int> indices;
            std::vector<Vertex> vertices;
            std::string currentMaterial;
            std::string currentGroup;
            bool smoothing = false;
            std::unordered_map<std::string, std::string> materialLibs;
        };
        
        static bool ParseOBJFile(const std::string& filepath, OBJData& data);
        static Vector3 ParseVector3(const std::string& line);
        static void ParseFace(const std::string& line, OBJData& data);
        static Mesh CreateMeshFromOBJData(const OBJData& data);
        
        // Helper functions
        static std::vector<std::string> SplitString(const std::string& str, char delimiter);
        static std::string TrimString(const std::string& str);
        static bool IsValidFloat(const std::string& str);
        static bool StartsWith(const std::string& s, const char* prefix);
        static int ResolveIndex(int idx, int size);
        static void ComputeMissingNormals(OBJData& data);
    };
}
