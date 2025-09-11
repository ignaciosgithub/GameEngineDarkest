#include "WorldSettingsPanel.h"
#include "../../Physics/PhysicsWorld.h"
#include "../../Core/Logging/Logger.h"
#include "../../Rendering/Lighting/LightOcclusion.h"
#include <imgui.h>

namespace GameEngine {

WorldSettingsPanel::WorldSettingsPanel() = default;

WorldSettingsPanel::~WorldSettingsPanel() = default;

void WorldSettingsPanel::Update(World* /*world*/, float /*deltaTime*/) {
    if (!m_visible) return;
    
    if (ImGui::Begin("World Settings", &m_visible)) {
        if (m_physicsWorld) {
            DrawGravitySettings();
            ImGui::Separator();
            DrawPhysicsSettings();
            ImGui::Separator();
            DrawPerformanceSettings();
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Rendering / Shadows", ImGuiTreeNodeFlags_DefaultOpen)) {
                int mode = static_cast<int>(LightOcclusion::GetDefaultSoftShadowMode());
                const char* items[] = {"Off", "Fixed", "Adaptive"};
                if (ImGui::Combo("Soft Shadow Mode", &mode, items, 3)) {
                    LightOcclusion::SetDefaultSoftShadowMode(static_cast<LightOcclusion::SoftShadowMode>(mode));
                }
                int fixedSamples = LightOcclusion::GetDefaultFixedSampleCount();
                if (ImGui::SliderInt("Fixed Sample Count", &fixedSamples, 4, 16)) {
                    LightOcclusion::SetDefaultFixedSampleCount(fixedSamples);
                }
                ImGui::Text("New lights/occlusion instances will use these defaults");
            }
        } else {
            ImGui::Text("Physics World not available");
        }
    }
    ImGui::End();
}

void WorldSettingsPanel::DrawGravitySettings() {
    if (ImGui::CollapsingHeader("Gravity Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        Vector3 gravity = m_physicsWorld->GetGravity();
        float gravityArray[3] = {gravity.x, gravity.y, gravity.z};
        
        if (ImGui::DragFloat3("Gravity", gravityArray, 0.1f, -50.0f, 50.0f)) {
            m_physicsWorld->SetGravity(Vector3(gravityArray[0], gravityArray[1], gravityArray[2]));
        }
        
        ImGui::Text("Current Gravity: (%.2f, %.2f, %.2f)", gravity.x, gravity.y, gravity.z);
        
        if (ImGui::Button("Reset to Earth Gravity")) {
            m_physicsWorld->SetGravity(Vector3(0.0f, -9.81f, 0.0f));
        }
        ImGui::SameLine();
        if (ImGui::Button("Zero Gravity")) {
            m_physicsWorld->SetGravity(Vector3(0.0f, 0.0f, 0.0f));
        }
        ImGui::SameLine();
        if (ImGui::Button("Moon Gravity")) {
            m_physicsWorld->SetGravity(Vector3(0.0f, -1.62f, 0.0f));
        }
    }
}

void WorldSettingsPanel::DrawPhysicsSettings() {
    if (ImGui::CollapsingHeader("Physics Settings")) {
        int maxSteps = m_physicsWorld->GetMaxPhysicsStepsPerFrame();
        if (ImGui::SliderInt("Max Physics Steps Per Frame", &maxSteps, 1, 20)) {
            m_physicsWorld->SetMaxPhysicsStepsPerFrame(maxSteps);
        }
        ImGui::Text("Prevents physics spiral of death by limiting substeps");
        
        bool enable2D = m_physicsWorld->IsEnable2DPhysics();
        if (ImGui::Checkbox("Enable 2D Physics", &enable2D)) {
            m_physicsWorld->SetEnable2DPhysics(enable2D);
        }
        ImGui::Text("Enables 2D physics simulation alongside 3D");
    }
}

void WorldSettingsPanel::DrawPerformanceSettings() {
    if (ImGui::CollapsingHeader("Performance Settings")) {
        bool useSpatialPartitioning = m_physicsWorld->GetUseSpatialPartitioning();
        if (ImGui::Checkbox("Use Spatial Partitioning", &useSpatialPartitioning)) {
            m_physicsWorld->SetUseSpatialPartitioning(useSpatialPartitioning);
        }
        ImGui::Text("Octree-based spatial partitioning for collision optimization");
        
        if (useSpatialPartitioning) {
            ImGui::Text("Spatial partitioning is ENABLED - Better performance for many objects");
        } else {
            ImGui::Text("Spatial partitioning is DISABLED - Brute force collision detection");
        }
    }
}

}
