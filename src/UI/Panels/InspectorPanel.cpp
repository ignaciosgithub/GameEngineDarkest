#include "InspectorPanel.h"
#include "../../Core/ECS/World.h"
#include "../../Core/Components/TransformComponent.h"
#include "../../Core/Components/CameraComponent.h"
#include "../../Core/Components/MovementComponent.h"
#include "../../Core/Components/MeshComponent.h"
#include "../../Core/Components/RigidBodyComponent.h"
#include "../../Core/Components/AudioComponent.h"
#include "../../Rendering/Lighting/Light.h"
#include "../../Core/Logging/Logger.h"
#include <imgui.h>

namespace GameEngine {

InspectorPanel::InspectorPanel() = default;

InspectorPanel::~InspectorPanel() = default;

void InspectorPanel::Update(World* world, float /*deltaTime*/) {
    if (!m_visible || !world) return;
    
    if (ImGui::Begin("Inspector", &m_visible)) {
        if (m_selectedEntity.IsValid() && world->IsEntityValid(m_selectedEntity)) {
            ImGui::Text("Entity ID: %d", m_selectedEntity.GetID());
            ImGui::Separator();
            
            DrawTransformComponent(world, m_selectedEntity);
            DrawMeshComponent(world, m_selectedEntity);
            DrawRigidBodyComponent(world, m_selectedEntity);
            DrawAudioComponent(world, m_selectedEntity);
            DrawLightComponent(world, m_selectedEntity);
            DrawCameraComponent(world, m_selectedEntity);
            DrawMovementComponent(world, m_selectedEntity);
            
            ImGui::Separator();
            if (ImGui::Button("Add Component")) {
                ImGui::OpenPopup("Add Component");
            }
        } else {
            ImGui::Text("No entity selected");
        }
        
        if (ImGui::BeginPopupModal("Add Component", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Select a component to add:");
            ImGui::Separator();
            
            if (!world->HasComponent<CameraComponent>(m_selectedEntity)) {
                if (ImGui::Button("Camera Component")) {
                    AddCameraComponent(world, m_selectedEntity);
                    ImGui::CloseCurrentPopup();
                }
            }
            
            if (!world->HasComponent<MovementComponent>(m_selectedEntity)) {
                if (ImGui::Button("Movement Component")) {
                    AddMovementComponent(world, m_selectedEntity);
                    ImGui::CloseCurrentPopup();
                }
            }
            
            if (!world->HasComponent<MeshComponent>(m_selectedEntity)) {
                if (ImGui::Button("Mesh Component")) {
                    AddMeshComponent(world, m_selectedEntity);
                    ImGui::CloseCurrentPopup();
                }
            }
            
            if (!world->HasComponent<RigidBodyComponent>(m_selectedEntity)) {
                if (ImGui::Button("RigidBody Component")) {
                    AddRigidBodyComponent(world, m_selectedEntity);
                    ImGui::CloseCurrentPopup();
                }
            }
            
            if (!world->HasComponent<AudioComponent>(m_selectedEntity)) {
                if (ImGui::Button("Audio Component")) {
                    AddAudioComponent(world, m_selectedEntity);
                    ImGui::CloseCurrentPopup();
                }
            }
            
            if (!world->HasComponent<LightComponent>(m_selectedEntity)) {
                if (ImGui::Button("Light Component")) {
                    AddLightComponent(world, m_selectedEntity);
                    ImGui::CloseCurrentPopup();
                }
            }
            
            ImGui::Separator();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void InspectorPanel::DrawTransformComponent(World* world, Entity entity) {
    auto* transform = world->GetComponent<TransformComponent>(entity);
    if (!transform) return;
    
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        Vector3 position = transform->transform.GetPosition();
        Vector3 rotation = transform->transform.GetRotation().ToEulerAngles();
        Vector3 scale = transform->transform.GetScale();
        
        if (ImGui::DragFloat3("Position", &position.x, 0.1f)) {
            transform->transform.SetPosition(position);
        }
        if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f)) {
            transform->transform.SetRotation(Quaternion::FromEulerAngles(rotation.x, rotation.y, rotation.z));
        }
        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f)) {
            transform->transform.SetScale(scale);
        }
    }
}

void InspectorPanel::DrawCameraComponent(World* world, Entity entity) {
    auto* camera = world->GetComponent<CameraComponent>(entity);
    if (!camera) return;
    
    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::SameLine();
        if (ImGui::Button("Remove##Camera")) {
            RemoveCameraComponent(world, entity);
            return;
        }
        
        float fov = camera->fieldOfView;
        float nearPlane = camera->nearPlane;
        float farPlane = camera->farPlane;
        
        if (ImGui::SliderFloat("Field of View", &fov, 10.0f, 170.0f)) {
            camera->SetFOV(fov);
        }
        if (ImGui::DragFloat("Near Plane", &nearPlane, 0.01f, 0.01f, 100.0f)) {
            camera->nearPlane = nearPlane;
        }
        if (ImGui::DragFloat("Far Plane", &farPlane, 1.0f, 1.0f, 10000.0f)) {
            camera->farPlane = farPlane;
        }
    }
}

void InspectorPanel::DrawMovementComponent(World* world, Entity entity) {
    auto* movement = world->GetComponent<MovementComponent>(entity);
    if (!movement) return;
    
    if (ImGui::CollapsingHeader("Movement")) {
        ImGui::SameLine();
        if (ImGui::Button("Remove##Movement")) {
            RemoveMovementComponent(world, entity);
            return;
        }
        
        ImGui::DragFloat("Movement Speed", &movement->movementSpeed, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat("Mouse Sensitivity", &movement->mouseSensitivity, 0.1f, 0.1f, 10.0f);
        
        ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", 
                   movement->velocity.x, movement->velocity.y, movement->velocity.z);
        ImGui::Text("Pitch: %.2f, Yaw: %.2f", movement->pitch, movement->yaw);
    }
}

void InspectorPanel::DrawMeshComponent(World* world, Entity entity) {
    auto* mesh = world->GetComponent<MeshComponent>(entity);
    if (!mesh) return;
    
    if (ImGui::CollapsingHeader("Mesh Component")) {
        ImGui::SameLine();
        if (ImGui::Button("Remove##Mesh")) {
            RemoveMeshComponent(world, entity);
            return;
        }
        
        bool visible = mesh->IsVisible();
        if (ImGui::Checkbox("Visible", &visible)) {
            mesh->SetVisible(visible);
        }
        
        ImGui::Text("Mesh Type: %s", mesh->GetMeshType().c_str());
        
        Vector3 color = mesh->GetColor();
        float colorArray[3] = {color.x, color.y, color.z};
        if (ImGui::ColorEdit3("Color", colorArray)) {
            mesh->SetColor(Vector3(colorArray[0], colorArray[1], colorArray[2]));
        }
        
        float metallic = mesh->GetMetallic();
        if (ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f)) {
            mesh->SetMetallic(metallic);
        }
        
        float roughness = mesh->GetRoughness();
        if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f)) {
            mesh->SetRoughness(roughness);
        }
        
        if (ImGui::Button("Load OBJ...")) {
            ImGui::OpenPopup("Load OBJ");
        }
        
        if (ImGui::BeginPopup("Load OBJ")) {
            static char objPath[256] = "";
            ImGui::InputText("OBJ Path", objPath, sizeof(objPath));
            if (ImGui::Button("Load") && strlen(objPath) > 0) {
                mesh->LoadMeshFromOBJ(objPath);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

void InspectorPanel::DrawRigidBodyComponent(World* world, Entity entity) {
    auto* rigidBody = world->GetComponent<RigidBodyComponent>(entity);
    if (!rigidBody) return;
    
    if (ImGui::CollapsingHeader("RigidBody Component")) {
        ImGui::SameLine();
        if (ImGui::Button("Remove##RigidBody")) {
            RemoveRigidBodyComponent(world, entity);
            return;
        }
        
        ImGui::Text("RigidBody: %s", rigidBody->GetRigidBody() ? "Active" : "Inactive");
        
        if (ImGui::Button("Add RigidBody") && !rigidBody->GetRigidBody()) {
            Logger::Info("RigidBody initialization requested");
        }
        
        if (ImGui::Button("Remove RigidBody") && rigidBody->GetRigidBody()) {
            Logger::Info("RigidBody removal requested");
        }
    }
}

void InspectorPanel::DrawAudioComponent(World* world, Entity entity) {
    auto* audio = world->GetComponent<AudioComponent>(entity);
    if (!audio) return;
    
    if (ImGui::CollapsingHeader("Audio Component")) {
        ImGui::SameLine();
        if (ImGui::Button("Remove##Audio")) {
            RemoveAudioComponent(world, entity);
            return;
        }
        
        float volume = audio->GetVolume();
        if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f)) {
            audio->SetVolume(volume);
        }
        
        float pitch = audio->GetPitch();
        if (ImGui::SliderFloat("Pitch", &pitch, 0.1f, 3.0f)) {
            audio->SetPitch(pitch);
        }
        
        bool looping = audio->IsLooping();
        if (ImGui::Checkbox("Looping", &looping)) {
            audio->SetLooping(looping);
        }
        
        bool playOnAwake = audio->GetPlayOnAwake();
        if (ImGui::Checkbox("Play On Awake", &playOnAwake)) {
            audio->SetPlayOnAwake(playOnAwake);
        }
        
        bool spatial = audio->IsSpatial();
        if (ImGui::Checkbox("3D Spatial", &spatial)) {
            audio->SetSpatial(spatial);
        }
        
        if (spatial) {
            float minDistance = audio->GetMinDistance();
            if (ImGui::DragFloat("Min Distance", &minDistance, 0.1f, 0.0f, 1000.0f)) {
                audio->SetMinDistance(minDistance);
            }
            
            float maxDistance = audio->GetMaxDistance();
            if (ImGui::DragFloat("Max Distance", &maxDistance, 1.0f, minDistance, 1000.0f)) {
                audio->SetMaxDistance(maxDistance);
            }
            
            float rolloff = audio->GetRolloffFactor();
            if (ImGui::SliderFloat("Rolloff Factor", &rolloff, 0.0f, 10.0f)) {
                audio->SetRolloffFactor(rolloff);
            }
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("Play")) {
            audio->Play();
        }
        ImGui::SameLine();
        if (ImGui::Button("Pause")) {
            audio->Pause();
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            audio->Stop();
        }
        
        ImGui::Text("State: %s", 
                   audio->IsPlaying() ? "Playing" : 
                   audio->IsPaused() ? "Paused" : "Stopped");
    }
}

void InspectorPanel::DrawLightComponent(World* world, Entity entity) {
    auto* lightComp = world->GetComponent<LightComponent>(entity);
    if (!lightComp) return;
    
    if (ImGui::CollapsingHeader("Light Component")) {
        ImGui::SameLine();
        if (ImGui::Button("Remove##Light")) {
            RemoveLightComponent(world, entity);
            return;
        }
        
        Light& light = lightComp->light;
        
        const char* lightTypes[] = {"Directional", "Point", "Spot"};
        int currentType = static_cast<int>(light.GetType());
        if (ImGui::Combo("Light Type", &currentType, lightTypes, 3)) {
            light.SetType(static_cast<LightType>(currentType));
        }
        
        Vector3 color = light.GetColor();
        float colorArray[3] = {color.x, color.y, color.z};
        if (ImGui::ColorEdit3("Color", colorArray)) {
            light.SetColor(Vector3(colorArray[0], colorArray[1], colorArray[2]));
        }
        
        float intensity = light.GetIntensity();
        if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100.0f)) {
            light.SetIntensity(intensity);
        }
        
        if (light.GetType() == LightType::Point || light.GetType() == LightType::Spot) {
            float range = light.GetRange();
            if (ImGui::DragFloat("Range", &range, 0.5f, 0.1f, 1000.0f)) {
                light.SetRange(range);
            }
        }
        
        if (light.GetType() == LightType::Spot) {
            float innerAngle = light.GetInnerConeAngle();
            float outerAngle = light.GetOuterConeAngle();
            
            if (ImGui::DragFloat("Inner Cone Angle", &innerAngle, 1.0f, 0.0f, 90.0f)) {
                light.SetSpotAngles(innerAngle, outerAngle);
            }
            
            if (ImGui::DragFloat("Outer Cone Angle", &outerAngle, 1.0f, innerAngle, 90.0f)) {
                light.SetSpotAngles(innerAngle, outerAngle);
            }
        }
        
        if (light.GetType() == LightType::Directional) {
            Vector3 direction = light.GetDirection();
            float dirArray[3] = {direction.x, direction.y, direction.z};
            if (ImGui::DragFloat3("Direction", dirArray, 0.01f, -1.0f, 1.0f)) {
                light.SetDirection(Vector3(dirArray[0], dirArray[1], dirArray[2]));
            }
        }
        
        ImGui::Separator();
        ImGui::Text("Shadow Settings");
        
        bool castShadows = light.GetCastShadows();
        if (ImGui::Checkbox("Cast Shadows", &castShadows)) {
            light.SetCastShadows(castShadows);
        }
        
        if (castShadows) {
            float shadowBias = light.GetShadowBias();
            if (ImGui::DragFloat("Shadow Bias", &shadowBias, 0.0001f, 0.0f, 0.1f, "%.4f")) {
                light.SetShadowBias(shadowBias);
            }
            
            int shadowMapSize = light.GetShadowMapSize();
            const char* shadowSizes[] = {"512", "1024", "2048", "4096"};
            int sizeIndex = 0;
            if (shadowMapSize == 512) sizeIndex = 0;
            else if (shadowMapSize == 1024) sizeIndex = 1;
            else if (shadowMapSize == 2048) sizeIndex = 2;
            else if (shadowMapSize == 4096) sizeIndex = 3;
            
            if (ImGui::Combo("Shadow Map Size", &sizeIndex, shadowSizes, 4)) {
                int newSize = 512;
                if (sizeIndex == 1) newSize = 1024;
                else if (sizeIndex == 2) newSize = 2048;
                else if (sizeIndex == 3) newSize = 4096;
                light.SetShadowMapSize(newSize);
            }
        }
    }
}

void InspectorPanel::AddCameraComponent(World* world, Entity entity) {
    if (world && entity.IsValid() && !world->HasComponent<CameraComponent>(entity)) {
        world->AddComponent<CameraComponent>(entity, 60.0f);
        Logger::Info("Added CameraComponent to entity: " + std::to_string(entity.GetID()));
    }
}

void InspectorPanel::AddMovementComponent(World* world, Entity entity) {
    if (world && entity.IsValid() && !world->HasComponent<MovementComponent>(entity)) {
        world->AddComponent<MovementComponent>(entity, 5.0f, 2.0f);
        Logger::Info("Added MovementComponent to entity: " + std::to_string(entity.GetID()));
    }
}

void InspectorPanel::AddMeshComponent(World* world, Entity entity) {
    if (world && entity.IsValid() && !world->HasComponent<MeshComponent>(entity)) {
        world->AddComponent<MeshComponent>(entity, "cube");
        Logger::Info("Added MeshComponent to entity: " + std::to_string(entity.GetID()));
    }
}

void InspectorPanel::AddRigidBodyComponent(World* world, Entity entity) {
    if (world && entity.IsValid() && !world->HasComponent<RigidBodyComponent>(entity)) {
        world->AddComponent<RigidBodyComponent>(entity);
        Logger::Info("Added RigidBodyComponent to entity: " + std::to_string(entity.GetID()));
    }
}

void InspectorPanel::AddAudioComponent(World* world, Entity entity) {
    if (world && entity.IsValid() && !world->HasComponent<AudioComponent>(entity)) {
        world->AddComponent<AudioComponent>(entity);
        Logger::Info("Added AudioComponent to entity: " + std::to_string(entity.GetID()));
    }
}

void InspectorPanel::AddLightComponent(World* world, Entity entity) {
    if (world && entity.IsValid() && !world->HasComponent<LightComponent>(entity)) {
        world->AddComponent<LightComponent>(entity, LightType::Point);
        Logger::Info("Added LightComponent to entity: " + std::to_string(entity.GetID()));
    }
}

void InspectorPanel::RemoveCameraComponent(World* world, Entity entity) {
    if (world && entity.IsValid() && world->HasComponent<CameraComponent>(entity)) {
        world->RemoveComponent<CameraComponent>(entity);
        Logger::Info("Removed CameraComponent from entity: " + std::to_string(entity.GetID()));
    }
}

void InspectorPanel::RemoveMovementComponent(World* world, Entity entity) {
    if (world && entity.IsValid() && world->HasComponent<MovementComponent>(entity)) {
        world->RemoveComponent<MovementComponent>(entity);
        Logger::Info("Removed MovementComponent from entity: " + std::to_string(entity.GetID()));
    }
}

void InspectorPanel::RemoveMeshComponent(World* world, Entity entity) {
    if (world && entity.IsValid() && world->HasComponent<MeshComponent>(entity)) {
        world->RemoveComponent<MeshComponent>(entity);
        Logger::Info("Removed MeshComponent from entity: " + std::to_string(entity.GetID()));
    }
}

void InspectorPanel::RemoveRigidBodyComponent(World* world, Entity entity) {
    if (world && entity.IsValid() && world->HasComponent<RigidBodyComponent>(entity)) {
        world->RemoveComponent<RigidBodyComponent>(entity);
        Logger::Info("Removed RigidBodyComponent from entity: " + std::to_string(entity.GetID()));
    }
}

void InspectorPanel::RemoveAudioComponent(World* world, Entity entity) {
    if (world && entity.IsValid() && world->HasComponent<AudioComponent>(entity)) {
        world->RemoveComponent<AudioComponent>(entity);
        Logger::Info("Removed AudioComponent from entity: " + std::to_string(entity.GetID()));
    }
}

void InspectorPanel::RemoveLightComponent(World* world, Entity entity) {
    if (world && entity.IsValid() && world->HasComponent<LightComponent>(entity)) {
        world->RemoveComponent<LightComponent>(entity);
        Logger::Info("Removed LightComponent from entity: " + std::to_string(entity.GetID()));
    }
}

}
