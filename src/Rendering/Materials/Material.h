#pragma once

#include "../Core/Texture.h"
#include "../Shaders/Shader.h"
#include "../../Core/Math/Vector2.h"
#include "../../Core/Math/Vector3.h"
#include "../../Core/Math/Vector4.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace GameEngine {

enum class MaterialType {
    Standard,
    Unlit,
    Transparent,
    Emissive
};

enum class BlendMode {
    Opaque,
    AlphaBlend,
    Additive,
    Multiply
};

struct MaterialProperties {
    // PBR Properties
    Vector3 albedo = Vector3(1.0f, 1.0f, 1.0f);
    float metallic = 0.0f;
    float roughness = 0.5f;
    float normalScale = 1.0f;
    float occlusionStrength = 1.0f;
    Vector3 emission = Vector3(0.0f, 0.0f, 0.0f);
    float emissionIntensity = 1.0f;
    
    // Transparency
    float alpha = 1.0f;
    BlendMode blendMode = BlendMode::Opaque;
    
    // Additional Properties
    bool doubleSided = false;
    bool receiveShadows = true;
    bool castShadows = true;
    
    // Texture UV tiling and offset
    Vector2 mainTextureScale = Vector2(1.0f, 1.0f);
    Vector2 mainTextureOffset = Vector2(0.0f, 0.0f);
};

class Material {
public:
    Material(const std::string& name = "DefaultMaterial");
    ~Material();
    
    // Material setup
    void SetShader(std::shared_ptr<Shader> shader);
    std::shared_ptr<Shader> GetShader() const { return m_shader; }
    
    void SetType(MaterialType type) { m_type = type; }
    MaterialType GetType() const { return m_type; }
    
    // Property setters
    void SetAlbedo(const Vector3& albedo) { m_properties.albedo = albedo; }
    void SetMetallic(float metallic) { m_properties.metallic = metallic; }
    void SetRoughness(float roughness) { m_properties.roughness = roughness; }
    void SetNormalScale(float scale) { m_properties.normalScale = scale; }
    void SetOcclusionStrength(float strength) { m_properties.occlusionStrength = strength; }
    void SetEmission(const Vector3& emission) { m_properties.emission = emission; }
    void SetEmissionIntensity(float intensity) { m_properties.emissionIntensity = intensity; }
    void SetAlpha(float alpha) { m_properties.alpha = alpha; }
    void SetBlendMode(BlendMode mode) { m_properties.blendMode = mode; }
    
    // Property getters
    const MaterialProperties& GetProperties() const { return m_properties; }
    MaterialProperties& GetProperties() { return m_properties; }
    
    // Texture management
    void SetTexture(const std::string& name, std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture(const std::string& name) const;
    bool HasTexture(const std::string& name) const;
    void RemoveTexture(const std::string& name);
    
    // Common texture setters
    void SetAlbedoTexture(std::shared_ptr<Texture> texture) { SetTexture("_MainTex", texture); }
    void SetNormalTexture(std::shared_ptr<Texture> texture) { SetTexture("_BumpMap", texture); }
    void SetMetallicTexture(std::shared_ptr<Texture> texture) { SetTexture("_MetallicGlossMap", texture); }
    void SetRoughnessTexture(std::shared_ptr<Texture> texture) { SetTexture("_RoughnessMap", texture); }
    void SetOcclusionTexture(std::shared_ptr<Texture> texture) { SetTexture("_OcclusionMap", texture); }
    void SetEmissionTexture(std::shared_ptr<Texture> texture) { SetTexture("_EmissionMap", texture); }
    
    // Common texture getters
    std::shared_ptr<Texture> GetAlbedoTexture() const { return GetTexture("_MainTex"); }
    std::shared_ptr<Texture> GetNormalTexture() const { return GetTexture("_BumpMap"); }
    std::shared_ptr<Texture> GetMetallicTexture() const { return GetTexture("_MetallicGlossMap"); }
    std::shared_ptr<Texture> GetRoughnessTexture() const { return GetTexture("_RoughnessMap"); }
    std::shared_ptr<Texture> GetOcclusionTexture() const { return GetTexture("_OcclusionMap"); }
    std::shared_ptr<Texture> GetEmissionTexture() const { return GetTexture("_EmissionMap"); }
    
    // Material binding for rendering
    void Bind() const;
    void Unbind() const;
    
    // Utility
    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }
    
    // Create default materials
    static std::shared_ptr<Material> CreateDefaultMaterial();
    static std::shared_ptr<Material> CreateUnlitMaterial();
    static std::shared_ptr<Material> CreateEmissiveMaterial();
    
private:
    void ApplyPropertiesToShader() const;
    void BindTextures() const;
    void BindTextureSlot(const std::string& textureName, int slot) const;
    void SetBlendMode() const;
    
    std::string m_name;
    MaterialType m_type = MaterialType::Standard;
    MaterialProperties m_properties;
    
    std::shared_ptr<Shader> m_shader;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    
    // Texture slot management
    mutable int m_currentTextureSlot = 0;
};

}
