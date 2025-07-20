#include "Material.h"
#include "../../Core/Logging/Logger.h"
#include "../../Core/Math/Vector2.h"
#include "../../Core/Math/Vector4.h"
#include "../Core/OpenGLHeaders.h"

namespace GameEngine {

Material::Material(const std::string& name) : m_name(name) {
    Logger::Info("Material created: " + m_name);
}

Material::~Material() {
    Logger::Debug("Material destroyed: " + m_name);
}

void Material::SetShader(std::shared_ptr<Shader> shader) {
    m_shader = shader;
    Logger::Debug("Shader set for material: " + m_name);
}

void Material::SetTexture(const std::string& name, std::shared_ptr<Texture> texture) {
    if (texture) {
        m_textures[name] = texture;
        Logger::Debug("Texture '" + name + "' set for material: " + m_name);
    } else {
        RemoveTexture(name);
    }
}

std::shared_ptr<Texture> Material::GetTexture(const std::string& name) const {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    return nullptr;
}

bool Material::HasTexture(const std::string& name) const {
    return m_textures.find(name) != m_textures.end();
}

void Material::RemoveTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        m_textures.erase(it);
        Logger::Debug("Texture '" + name + "' removed from material: " + m_name);
    }
}

void Material::Bind() const {
    if (!m_shader) {
        Logger::Warning("No shader set for material: " + m_name);
        return;
    }
    
    m_shader->Use();
    
    ApplyPropertiesToShader();
    
    BindTextures();
    
    SetBlendMode();
    
    Logger::Debug("Material bound: " + m_name);
}

void Material::Unbind() const {
    if (m_shader) {
        m_shader->Unuse();
    }
    
    m_currentTextureSlot = 0;
    
    Logger::Debug("Material unbound: " + m_name);
}

void Material::ApplyPropertiesToShader() const {
    if (!m_shader) return;
    
    m_shader->SetVector3("_Albedo", m_properties.albedo);
    m_shader->SetFloat("_Metallic", m_properties.metallic);
    m_shader->SetFloat("_Roughness", m_properties.roughness);
    m_shader->SetFloat("_NormalScale", m_properties.normalScale);
    m_shader->SetFloat("_OcclusionStrength", m_properties.occlusionStrength);
    m_shader->SetVector3("_Emission", m_properties.emission);
    m_shader->SetFloat("_EmissionIntensity", m_properties.emissionIntensity);
    m_shader->SetFloat("_Alpha", m_properties.alpha);
    
    Vector4 textureST(m_properties.mainTextureScale.x, m_properties.mainTextureScale.y, 
                      m_properties.mainTextureOffset.x, m_properties.mainTextureOffset.y);
    m_shader->SetVector4("_MainTex_ST", textureST);
    
    m_shader->SetBool("_DoubleSided", m_properties.doubleSided);
    m_shader->SetBool("_ReceiveShadows", m_properties.receiveShadows);
    m_shader->SetBool("_CastShadows", m_properties.castShadows);
}

void Material::BindTextures() const {
    m_currentTextureSlot = 0;
    
    BindTextureSlot("_MainTex", 0);
    BindTextureSlot("_BumpMap", 1);
    BindTextureSlot("_MetallicGlossMap", 2);
    BindTextureSlot("_RoughnessMap", 3);
    BindTextureSlot("_OcclusionMap", 4);
    BindTextureSlot("_EmissionMap", 5);
    
    int slot = 6;
    for (const auto& pair : m_textures) {
        const std::string& textureName = pair.first;
        
        if (textureName == "_MainTex" || textureName == "_BumpMap" || 
            textureName == "_MetallicGlossMap" || textureName == "_RoughnessMap" ||
            textureName == "_OcclusionMap" || textureName == "_EmissionMap") {
            continue;
        }
        
        BindTextureSlot(textureName, slot++);
    }
}

void Material::BindTextureSlot(const std::string& textureName, int slot) const {
    auto texture = GetTexture(textureName);
    if (texture && m_shader) {
        texture->Bind(slot);
        m_shader->SetInt(textureName, slot);
        Logger::Debug("Texture '" + textureName + "' bound to slot " + std::to_string(slot));
    }
}

void Material::SetBlendMode() const {
    Logger::Debug("Setting blend mode (simplified)");
    
    switch (m_properties.blendMode) {
        case BlendMode::Opaque:
            Logger::Debug("Blend mode: Opaque");
            break;
        case BlendMode::AlphaBlend:
            Logger::Debug("Blend mode: Alpha Blend");
            break;
        case BlendMode::Additive:
            Logger::Debug("Blend mode: Additive");
            break;
        case BlendMode::Multiply:
            Logger::Debug("Blend mode: Multiply");
            break;
    }
}

std::shared_ptr<Material> Material::CreateDefaultMaterial() {
    auto material = std::make_shared<Material>("DefaultMaterial");
    material->SetType(MaterialType::Standard);
    
    material->SetAlbedo(Vector3(0.8f, 0.8f, 0.8f));
    material->SetMetallic(0.0f);
    material->SetRoughness(0.5f);
    material->SetAlpha(1.0f);
    
    Logger::Info("Default material created");
    return material;
}

std::shared_ptr<Material> Material::CreateUnlitMaterial() {
    auto material = std::make_shared<Material>("UnlitMaterial");
    material->SetType(MaterialType::Unlit);
    
    material->SetAlbedo(Vector3(1.0f, 1.0f, 1.0f));
    material->SetAlpha(1.0f);
    
    Logger::Info("Unlit material created");
    return material;
}

std::shared_ptr<Material> Material::CreateEmissiveMaterial() {
    auto material = std::make_shared<Material>("EmissiveMaterial");
    material->SetType(MaterialType::Emissive);
    
    material->SetAlbedo(Vector3(0.0f, 0.0f, 0.0f));
    material->SetEmission(Vector3(1.0f, 1.0f, 1.0f));
    material->SetEmissionIntensity(2.0f);
    material->SetAlpha(1.0f);
    
    Logger::Info("Emissive material created");
    return material;
}

}
