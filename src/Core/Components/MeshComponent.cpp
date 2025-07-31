#include "MeshComponent.h"
#include "../Logging/Logger.h"

namespace GameEngine {
    MeshComponent::MeshComponent() {
        SetMesh("cube");
    }
    
    MeshComponent::MeshComponent(const std::string& meshType) {
        SetMesh(meshType);
    }
    
    MeshComponent::MeshComponent(std::shared_ptr<Mesh> mesh) 
        : m_mesh(mesh), m_meshType("custom") {
        if (!m_mesh) {
            Logger::Warning("MeshComponent created with null mesh, defaulting to cube");
            SetMesh("cube");
        }
    }
    
    void MeshComponent::SetMesh(std::shared_ptr<Mesh> mesh) {
        if (!mesh) {
            Logger::Warning("Attempted to set null mesh, ignoring");
            return;
        }
        
        m_mesh = mesh;
        m_meshType = "custom";
        Logger::Debug("MeshComponent mesh set to custom mesh");
    }
    
    void MeshComponent::SetMesh(const std::string& meshType) {
        m_meshType = meshType;
        CreateMeshFromType(meshType);
    }
    
    void MeshComponent::LoadMeshFromOBJ(const std::string& filepath) {
        try {
            auto loadedMesh = std::make_shared<Mesh>(Mesh::LoadFromOBJ(filepath));
            
            if (loadedMesh && loadedMesh->GetVertices().size() > 0) {
                m_mesh = loadedMesh;
                m_meshType = "obj:" + filepath;
                Logger::Info("Successfully loaded OBJ mesh from: " + filepath);
            } else {
                Logger::Error("Failed to load OBJ mesh from: " + filepath + ", keeping current mesh");
            }
        }
        catch (const std::exception& e) {
            Logger::Error("Exception loading OBJ mesh from " + filepath + ": " + e.what());
        }
    }
    
    void MeshComponent::CreateMeshFromType(const std::string& meshType) {
        try {
            if (meshType == "cube") {
                m_mesh = std::make_shared<Mesh>(Mesh::CreateCube(1.0f));
                Logger::Debug("Created cube mesh for MeshComponent");
            }
            else if (meshType == "sphere") {
                m_mesh = std::make_shared<Mesh>(Mesh::CreateSphere(1.0f, 32));
                Logger::Debug("Created sphere mesh for MeshComponent");
            }
            else if (meshType == "plane") {
                m_mesh = std::make_shared<Mesh>(Mesh::CreatePlane(1.0f, 1.0f));
                Logger::Debug("Created plane mesh for MeshComponent");
            }
            else if (meshType.substr(0, 4) == "obj:") {
                std::string filepath = meshType.substr(4);
                LoadMeshFromOBJ(filepath);
                return; // LoadMeshFromOBJ handles logging
            }
            else {
                Logger::Warning("Unknown mesh type: " + meshType + ", defaulting to cube");
                m_mesh = std::make_shared<Mesh>(Mesh::CreateCube(1.0f));
                m_meshType = "cube";
            }
            
            if (m_mesh && !m_mesh->IsUploaded()) {
                m_mesh->Upload();
            }
        }
        catch (const std::exception& e) {
            Logger::Error("Exception creating mesh of type " + meshType + ": " + e.what());
            m_mesh = std::make_shared<Mesh>(Mesh::CreateCube(1.0f));
            m_meshType = "cube";
            if (m_mesh && !m_mesh->IsUploaded()) {
                m_mesh->Upload();
            }
        }
    }
}
