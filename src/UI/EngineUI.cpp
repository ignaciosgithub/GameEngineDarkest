#include "EngineUI.h"
#include "ImGuiRenderer.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/ProjectPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/WorldSettingsPanel.h"
#include "../Core/Logging/Logger.h"
#include "../Core/Editor/PlayModeManager.h"
#include "../Core/Editor/SelectionManager.h"
#include "../Core/Scenes/Scene.h"
#include "../Core/Project/ProjectManager.h"
#include "../Core/Components/TransformComponent.h"
#include "../Core/Components/MeshComponent.h"
#include "../Rendering/Loaders/OBJLoader.h"
#include <imgui.h>

namespace GameEngine {

EngineUI::EngineUI() = default;

EngineUI::~EngineUI() {
    Shutdown();
}

bool EngineUI::Initialize(GLFWwindow* window) {
    m_imguiRenderer = std::make_unique<ImGuiRenderer>();
    if (!m_imguiRenderer->Initialize(window)) {
        Logger::Error("Failed to initialize ImGui renderer");
        return false;
    }
    
    m_panels.push_back(std::make_unique<SceneHierarchyPanel>());
    m_panels.push_back(std::make_unique<InspectorPanel>());
    m_panels.push_back(std::make_unique<ViewportPanel>());
    m_panels.push_back(std::make_unique<ProjectPanel>());
    m_panels.push_back(std::make_unique<ConsolePanel>());
    m_panels.push_back(std::make_unique<WorldSettingsPanel>());
    
    ResetPanelVisibility();
    Logger::Info("Engine UI initialized successfully");
    return true;
}

void EngineUI::Shutdown() {
    m_panels.clear();
    
    if (m_imguiRenderer) {
        m_imguiRenderer->Shutdown();
        m_imguiRenderer.reset();
    }
    
    Logger::Info("Engine UI shutdown");
}

void EngineUI::Update(World* world, float deltaTime) {
    m_world = world;
    
    if (!m_imguiRenderer) return;
    
    m_imguiRenderer->BeginFrame();
    
    RenderDockSpace();
    RenderMainMenuBar();
    
    SceneHierarchyPanel* hierarchyPanel = nullptr;
    InspectorPanel* inspectorPanel = nullptr;
    
    for (auto& panel : m_panels) {
        if (auto* hierarchy = dynamic_cast<SceneHierarchyPanel*>(panel.get())) {
            hierarchyPanel = hierarchy;
        }
        if (auto* inspector = dynamic_cast<InspectorPanel*>(panel.get())) {
            inspectorPanel = inspector;
        }
        if (auto* viewport = dynamic_cast<ViewportPanel*>(panel.get())) {
            viewport->SetPlayModeManager(m_playModeManager);
            viewport->SetSelectionManager(m_selectionManager);
        }
        if (auto* worldSettings = dynamic_cast<WorldSettingsPanel*>(panel.get())) {
            worldSettings->SetPhysicsWorld(m_physicsWorld);
        }
        panel->Update(world, deltaTime);
    }
    
    if (hierarchyPanel && inspectorPanel) {
        Entity selectedEntity = hierarchyPanel->GetSelectedEntity();
        inspectorPanel->SetSelectedEntity(selectedEntity);
    }
    
    if (m_showDemoWindow) {
        Logger::Debug("Demo window would be shown");
    }
    
    if (m_showMetricsWindow) {
        Logger::Debug("Metrics window would be shown");
    }
}

void EngineUI::Render() {
    if (m_imguiRenderer) {
        m_imguiRenderer->Render();
    }
}

void EngineUI::RenderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) {
                CreateNewScene();
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Save Project", "Ctrl+S")) {
                SaveProject();
            }
            
            ImGui::Separator();
            
            static char sceneName[256] = "Untitled Scene";
            ImGui::InputText("Scene Name", sceneName, sizeof(sceneName));
            
            if (ImGui::MenuItem("Save Scene")) {
                SaveCurrentScene(std::string(sceneName));
            }
            
            if (ImGui::MenuItem("Save Scene As...")) {
                SaveSceneAs(std::string(sceneName));
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Import Asset...")) {
                Logger::Info("Import Asset requested");
            }
            
            if (ImGui::MenuItem("Import OBJ...")) {
                ImGui::OpenPopup("Import OBJ File");
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Window")) {
            for (auto& panel : m_panels) {
                if (auto* hierarchy = dynamic_cast<SceneHierarchyPanel*>(panel.get())) {
                    ImGui::MenuItem("Scene Hierarchy", nullptr, &hierarchy->IsVisible());
                }
                if (auto* inspector = dynamic_cast<InspectorPanel*>(panel.get())) {
                    ImGui::MenuItem("Inspector", nullptr, &inspector->IsVisible());
                }
                if (auto* viewport = dynamic_cast<ViewportPanel*>(panel.get())) {
                    ImGui::MenuItem("Viewport", nullptr, &viewport->IsVisible());
                }
                if (auto* project = dynamic_cast<ProjectPanel*>(panel.get())) {
                    ImGui::MenuItem("Project", nullptr, &project->IsVisible());
                }
                if (auto* console = dynamic_cast<ConsolePanel*>(panel.get())) {
                    ImGui::MenuItem("Console", nullptr, &console->IsVisible());
                }
                if (auto* worldSettings = dynamic_cast<WorldSettingsPanel*>(panel.get())) {
                    ImGui::MenuItem("World Settings", nullptr, &worldSettings->IsVisible());
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Reset Panel Layout")) {
                ResetPanelVisibility();
            }
            ImGui::EndMenu();
        }
        
        if (m_playModeManager) {
            ImGui::Separator();
            
            EditorMode currentMode = m_playModeManager->GetCurrentMode();
            
            if (currentMode == EditorMode::Edit) {
                if (ImGui::Button("▶ Play")) {
                    m_playModeManager->SwitchToPlayMode();
                }
            } else {
                if (ImGui::Button("⏹ Stop")) {
                    m_playModeManager->SwitchToEditMode();
                }
            }
            
            ImGui::SameLine();
            
            if (currentMode == EditorMode::Play) {
                if (ImGui::Button("⏸ Pause")) {
                    m_playModeManager->TogglePause();
                }
            } else if (currentMode == EditorMode::Paused) {
                if (ImGui::Button("▶ Resume")) {
                    m_playModeManager->TogglePause();
                }
            }
            
            ImGui::SameLine();
            std::string modeText = (currentMode == EditorMode::Edit ? "Edit Mode" : 
                                  currentMode == EditorMode::Play ? "Play Mode" : "Paused");
            ImGui::Text("%s", modeText.c_str());
        }
        
        ImGui::EndMainMenuBar();
    }
    
    if (ImGui::BeginPopupModal("Import OBJ File", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char objFilePath[512] = "";
        ImGui::Text("Select OBJ file to import:");
        ImGui::InputText("File Path", objFilePath, sizeof(objFilePath));
        
        if (ImGui::Button("Browse...")) {
            Logger::Info("File browser would open here");
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("Import") && strlen(objFilePath) > 0) {
            ImportOBJFile(objFilePath);
            ImGui::CloseCurrentPopup();
            memset(objFilePath, 0, sizeof(objFilePath)); // Clear the path
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            memset(objFilePath, 0, sizeof(objFilePath)); // Clear the path
        }
        
        ImGui::EndPopup();
    }
}

void EngineUI::RenderDockSpace() {
#ifdef IMGUI_HAS_DOCK
    static bool dockspaceOpen = true;
    
    if (dockspaceOpen) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        
        ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar(3);
        
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        
        ImGui::End();
    }
#else
    Logger::Debug("DockSpace not available - using fallback mode");
#endif
}

ViewportPanel* EngineUI::GetViewportPanel() const {
    for (const auto& panel : m_panels) {
        if (auto* viewportPanel = dynamic_cast<ViewportPanel*>(panel.get())) {
            return viewportPanel;
        }
    }
    return nullptr;
}

void EngineUI::CreateNewScene() {
    Logger::Info("Creating new scene");
}

void EngineUI::SaveCurrentScene(const std::string& sceneName) {
    Logger::Info("Saving current scene: " + sceneName);
    std::string filepath = "Scenes/" + sceneName + ".scene";
    Logger::Info("Scene would be saved to: " + filepath);
}

void EngineUI::SaveSceneAs(const std::string& sceneName) {
    Logger::Info("Save Scene As requested: " + sceneName);
}

void EngineUI::SaveProject() {
    if (ProjectManager::Instance().IsProjectLoaded()) {
        if (ProjectManager::Instance().SaveProject()) {
            Logger::Info("Project saved successfully");
        } else {
            Logger::Error("Failed to save project");
        }
    } else {
        Logger::Warning("No project loaded to save");
    }
}

void EngineUI::ResetPanelVisibility() {
    for (auto& panel : m_panels) {
        panel->IsVisible() = true;
    }
    Logger::Info("All UI panels visibility reset to visible");
}

void EngineUI::ImportOBJFile(const std::string& filePath) {
    if (!m_world) {
        Logger::Error("Cannot import OBJ: World not available");
        return;
    }
    
    Logger::Info("Importing OBJ file: " + filePath);
    
    Entity newEntity = m_world->CreateEntity();
    
    m_world->AddComponent<TransformComponent>(newEntity, Vector3(0, 0, 0));
    
    auto* meshComponent = m_world->AddComponent<MeshComponent>(newEntity, "imported_obj");
    if (meshComponent) {
        meshComponent->LoadMeshFromOBJ(filePath);
        if (meshComponent->HasMesh()) {
            Logger::Info("Successfully imported OBJ file as entity: " + std::to_string(newEntity.GetID()));
        } else {
            Logger::Error("Failed to load OBJ file: " + filePath);
            m_world->DestroyEntity(newEntity);
        }
    } else {
        Logger::Error("Failed to add MeshComponent to imported entity");
        m_world->DestroyEntity(newEntity);
    }
}

}
