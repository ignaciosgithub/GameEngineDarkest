#pragma once

#include "AnimationSystem.h"
#include "../Core/ECS/Component.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace GameEngine {
    class GameObject;
    class AnimationClip;
    
    class Animator : public Component<Animator> {
    public:
        Animator();
        ~Animator();
        
        // Animation playback control
        void Play(const std::string& animationName, float fadeTime = 0.0f);
        void Stop(const std::string& animationName);
        void Pause(const std::string& animationName);
        void Resume(const std::string& animationName);
        
        void StopAll();
        void PauseAll();
        void ResumeAll();
        
        // Animation state management
        void AddAnimationClip(std::shared_ptr<AnimationClip> clip);
        void RemoveAnimationClip(const std::string& name);
        bool HasAnimationClip(const std::string& name) const;
        
        // Animation properties
        void SetSpeed(const std::string& animationName, float speed);
        float GetSpeed(const std::string& animationName) const;
        
        void SetWeight(const std::string& animationName, float weight);
        float GetWeight(const std::string& animationName) const;
        
        void SetBlendMode(const std::string& animationName, AnimationBlendMode mode);
        AnimationBlendMode GetBlendMode(const std::string& animationName) const;
        
        // State queries
        bool IsPlaying(const std::string& animationName) const;
        bool IsPlaying() const; // Any animation playing
        float GetTime(const std::string& animationName) const;
        float GetNormalizedTime(const std::string& animationName) const;
        
        // Cross-fade and blending
        void CrossFade(const std::string& animationName, float fadeTime);
        void Blend(const std::string& animationName, float targetWeight, float fadeTime);
        
        // Update method for animation system
        void Update(float deltaTime);
        
        // Target GameObject
        void SetTarget(GameObject* target) { m_target = target; }
        GameObject* GetTarget() const { return m_target; }
        
        // Animation events
        using AnimationEventCallback = std::function<void(const std::string&)>;
        void SetAnimationEventCallback(AnimationEventCallback callback) { m_eventCallback = callback; }
        
    private:
        GameObject* m_target = nullptr;
        std::unordered_map<std::string, AnimationState> m_animationStates;
        AnimationEventCallback m_eventCallback;
        
        // Cross-fade state
        struct CrossFadeState {
            std::string fromAnimation;
            std::string toAnimation;
            float fadeTime;
            float currentTime;
            bool active = false;
        };
        CrossFadeState m_crossFadeState;
        
        void UpdateAnimationState(AnimationState& state, float deltaTime);
        void ApplyAnimation(const AnimationState& state);
        void ProcessCrossFade(float deltaTime);
        
        AnimationState* GetAnimationState(const std::string& name);
        const AnimationState* GetAnimationState(const std::string& name) const;
    };
}
