#pragma once

#include "../ECS/Component.h"
#include "../../Rendering/Meshes/Mesh.h"
#include <memory>
#include <string>

namespace GameEngine {
    class MeshComponent : public Component<MeshComponent> {
    public:
        MeshComponent();
        MeshComponent(const std::string& meshType);
        MeshComponent(std::shared_ptr<Mesh> mesh);
        ~MeshComponent() = default;
        
        // Mesh management
        void SetMesh(std::shared_ptr<Mesh> mesh);
        void SetMesh(const std::string& meshType);
        void LoadMeshFromOBJ(const std::string& filepath);
        
        std::shared_ptr<Mesh> GetMesh() const { return m_mesh; }
        bool HasMesh() const { return m_mesh != nullptr; }
        
        // Rendering properties
        bool IsVisible() const { return m_visible; }
        void SetVisible(bool visible) { m_visible = visible; }
        
        // Material properties (simplified)
        const Vector3& GetColor() const { return m_color; }
        void SetColor(const Vector3& color) { m_color = color; }
        
        float GetMetallic() const { return m_metallic; }
        void SetMetallic(float metallic) { m_metallic = metallic; }
        
        float GetRoughness() const { return m_roughness; }
        void SetRoughness(float roughness) { m_roughness = roughness; }
        
        // Mesh type for serialization
        const std::string& GetMeshType() const { return m_meshType; }
        
    private:
        std::shared_ptr<Mesh> m_mesh;
        std::string m_meshType;
        bool m_visible = true;
        
        // Basic material properties
        Vector3 m_color = Vector3(1.0f, 1.0f, 1.0f);
        float m_metallic = 0.0f;
        float m_roughness = 0.5f;
        
        void CreateMeshFromType(const std::string& meshType);
    };
}
