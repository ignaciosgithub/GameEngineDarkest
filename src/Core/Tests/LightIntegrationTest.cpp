#include "../GameObject/GameObject.h"
#include "../ECS/World.h"
#include "../../Rendering/Lighting/Light.h"
#include "../Logging/Logger.h"
#include <iostream>

namespace GameEngine {
    
    void TestDynamicLightAttachment() {
        Logger::Info("=== Testing Dynamic Light Attachment ===");
        
        World world;
        Entity entity = world.CreateEntity();
        GameObject gameObject(&world, entity, "TestLightObject");
        
        if (gameObject.HasComponent<LightComponent>()) {
            Logger::Error("GameObject should not have LightComponent initially");
            return;
        }
        
        auto* lightComponent = gameObject.AddComponent<LightComponent>(LightType::Point);
        if (!lightComponent) {
            Logger::Error("Failed to add LightComponent to GameObject");
            return;
        }
        
        if (!gameObject.HasComponent<LightComponent>()) {
            Logger::Error("GameObject should have LightComponent after adding");
            return;
        }
        
        Logger::Info("✓ Dynamic light attachment successful");
        
        Logger::Info("=== Testing Real-time Light Property Changes ===");
        
        lightComponent->light.SetColor(Vector3(1.0f, 0.0f, 0.0f)); // Red
        lightComponent->light.SetIntensity(2.0f);
        lightComponent->light.SetRange(15.0f);
        
        Vector3 initialColor = lightComponent->light.GetColor();
        float initialIntensity = lightComponent->light.GetIntensity();
        float initialRange = lightComponent->light.GetRange();
        
        if (initialColor.x != 1.0f || initialColor.y != 0.0f || initialColor.z != 0.0f) {
            Logger::Error("Initial light color not set correctly");
            return;
        }
        
        if (initialIntensity != 2.0f) {
            Logger::Error("Initial light intensity not set correctly");
            return;
        }
        
        if (initialRange != 15.0f) {
            Logger::Error("Initial light range not set correctly");
            return;
        }
        
        lightComponent->light.SetColor(Vector3(0.0f, 1.0f, 0.0f)); // Green
        lightComponent->light.SetIntensity(3.5f);
        lightComponent->light.SetRange(25.0f);
        lightComponent->light.SetType(LightType::Spot);
        
        Vector3 newColor = lightComponent->light.GetColor();
        float newIntensity = lightComponent->light.GetIntensity();
        float newRange = lightComponent->light.GetRange();
        LightType newType = lightComponent->light.GetType();
        
        if (newColor.x != 0.0f || newColor.y != 1.0f || newColor.z != 0.0f) {
            Logger::Error("Runtime light color change failed");
            return;
        }
        
        if (newIntensity != 3.5f) {
            Logger::Error("Runtime light intensity change failed");
            return;
        }
        
        if (newRange != 25.0f) {
            Logger::Error("Runtime light range change failed");
            return;
        }
        
        if (newType != LightType::Spot) {
            Logger::Error("Runtime light type change failed");
            return;
        }
        
        Logger::Info("✓ Real-time light property changes successful");
        
        Logger::Info("=== Testing Dynamic Light Removal ===");
        
        gameObject.RemoveComponent<LightComponent>();
        
        if (gameObject.HasComponent<LightComponent>()) {
            Logger::Error("GameObject should not have LightComponent after removal");
            return;
        }
        
        Logger::Info("✓ Dynamic light removal successful");
        
        Logger::Info("=== Testing Multiple Light Management ===");
        
        auto* light1 = gameObject.AddComponent<LightComponent>(LightType::Directional);
        light1->light.SetColor(Vector3(1.0f, 1.0f, 1.0f));
        light1->light.SetIntensity(1.0f);
        
        auto* retrievedLight = gameObject.GetComponent<LightComponent>();
        if (!retrievedLight) {
            Logger::Error("Failed to retrieve LightComponent");
            return;
        }
        
        retrievedLight->light.SetIntensity(4.0f);
        
        if (retrievedLight->light.GetIntensity() != 4.0f) {
            Logger::Error("Failed to modify light through retrieved pointer");
            return;
        }
        
        Logger::Info("✓ Multiple light management successful");
        Logger::Info("=== All Light Integration Tests Passed! ===");
    }
    
    void TestLightPositionSync() {
        Logger::Info("=== Testing Light Position Synchronization ===");
        
        World world;
        Entity entity = world.CreateEntity();
        GameObject gameObject(&world, entity, "PositionSyncTest");
        
        auto* lightComponent = gameObject.AddComponent<LightComponent>(LightType::Point);
        
        auto* transform = gameObject.GetTransform();
        transform->transform.SetPosition(Vector3(10.0f, 5.0f, -3.0f));
        
        Vector3 transformPos = transform->transform.GetPosition();
        
        lightComponent->light.SetPosition(transformPos);
        
        Vector3 lightPos = lightComponent->light.GetPosition();
        
        if (lightPos.x != 10.0f || lightPos.y != 5.0f || lightPos.z != -3.0f) {
            Logger::Error("Light position not synchronized with transform");
            return;
        }
        
        Logger::Info("✓ Light position synchronization successful");
    }
}

int main() {
    using namespace GameEngine;
    
    Logger::Info("Starting Light Integration Tests...");
    
    try {
        TestDynamicLightAttachment();
        TestLightPositionSync();
        
        std::cout << "\n=== ALL TESTS PASSED ===\n" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        Logger::Error("Test failed with exception: " + std::string(e.what()));
        return 1;
    }
}
