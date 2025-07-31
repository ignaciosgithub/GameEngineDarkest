#include "Animator.h"
#include "../Core/GameObject/GameObject.h"
#include "../Core/Logging/Logger.h"
#include <algorithm>

namespace GameEngine {
    
    Animator::Animator() = default;
    Animator::~Animator() = default;
    
    void Animator::Play(const std::string& animationName, float fadeTime) {
        auto* state = GetAnimationState(animationName);
        if (!state) {
            Logger::Error("Animation not found: " + animationName);
            return;
        }
        
        if (fadeTime > 0.0f) {
            CrossFade(animationName, fadeTime);
        } else {
            state->isPlaying = true;
            state->time = 0.0f;
            Logger::Debug("Playing animation: " + animationName);
        }
    }
    
    void Animator::Stop(const std::string& animationName) {
        auto* state = GetAnimationState(animationName);
        if (state) {
            state->isPlaying = false;
            state->time = 0.0f;
            Logger::Debug("Stopped animation: " + animationName);
        }
    }
    
    void Animator::Pause(const std::string& animationName) {
        auto* state = GetAnimationState(animationName);
        if (state) {
            state->isPlaying = false;
            Logger::Debug("Paused animation: " + animationName);
        }
    }
    
    void Animator::Resume(const std::string& animationName) {
        auto* state = GetAnimationState(animationName);
        if (state) {
            state->isPlaying = true;
            Logger::Debug("Resumed animation: " + animationName);
        }
    }
    
    void Animator::StopAll() {
        for (auto& pair : m_animationStates) {
            pair.second.isPlaying = false;
            pair.second.time = 0.0f;
        }
        Logger::Debug("Stopped all animations");
    }
    
    void Animator::PauseAll() {
        for (auto& pair : m_animationStates) {
            pair.second.isPlaying = false;
        }
        Logger::Debug("Paused all animations");
    }
    
    void Animator::ResumeAll() {
        for (auto& pair : m_animationStates) {
            if (pair.second.enabled) {
                pair.second.isPlaying = true;
            }
        }
        Logger::Debug("Resumed all animations");
    }
    
    void Animator::AddAnimationClip(std::shared_ptr<AnimationClip> clip) {
        if (!clip) {
            Logger::Error("Cannot add null animation clip");
            return;
        }
        
        AnimationState state(clip);
        m_animationStates[clip->GetName()] = state;
        Logger::Debug("Added animation clip: " + clip->GetName());
    }
    
    void Animator::RemoveAnimationClip(const std::string& name) {
        auto it = m_animationStates.find(name);
        if (it != m_animationStates.end()) {
            m_animationStates.erase(it);
            Logger::Debug("Removed animation clip: " + name);
        }
    }
    
    bool Animator::HasAnimationClip(const std::string& name) const {
        return m_animationStates.find(name) != m_animationStates.end();
    }
    
    void Animator::SetSpeed(const std::string& animationName, float speed) {
        auto* state = GetAnimationState(animationName);
        if (state) {
            state->speed = std::max(0.0f, speed);
        }
    }
    
    float Animator::GetSpeed(const std::string& animationName) const {
        const auto* state = GetAnimationState(animationName);
        return state ? state->speed : 0.0f;
    }
    
    void Animator::SetWeight(const std::string& animationName, float weight) {
        auto* state = GetAnimationState(animationName);
        if (state) {
            state->weight = std::max(0.0f, std::min(1.0f, weight));
        }
    }
    
    float Animator::GetWeight(const std::string& animationName) const {
        const auto* state = GetAnimationState(animationName);
        return state ? state->weight : 0.0f;
    }
    
    void Animator::SetBlendMode(const std::string& animationName, AnimationBlendMode mode) {
        auto* state = GetAnimationState(animationName);
        if (state) {
            state->blendMode = mode;
        }
    }
    
    AnimationBlendMode Animator::GetBlendMode(const std::string& animationName) const {
        const auto* state = GetAnimationState(animationName);
        return state ? state->blendMode : AnimationBlendMode::Override;
    }
    
    bool Animator::IsPlaying(const std::string& animationName) const {
        const auto* state = GetAnimationState(animationName);
        return state ? state->isPlaying : false;
    }
    
    bool Animator::IsPlaying() const {
        for (const auto& pair : m_animationStates) {
            if (pair.second.isPlaying) {
                return true;
            }
        }
        return false;
    }
    
    float Animator::GetTime(const std::string& animationName) const {
        const auto* state = GetAnimationState(animationName);
        return state ? state->time : 0.0f;
    }
    
    float Animator::GetNormalizedTime(const std::string& animationName) const {
        const auto* state = GetAnimationState(animationName);
        if (!state || !state->clip) {
            return 0.0f;
        }
        
        float length = state->clip->GetLength();
        return length > 0.0f ? state->time / length : 0.0f;
    }
    
    void Animator::CrossFade(const std::string& animationName, float fadeTime) {
        auto* state = GetAnimationState(animationName);
        if (!state) {
            Logger::Error("Animation not found for cross-fade: " + animationName);
            return;
        }
        
        std::string currentAnimation;
        for (const auto& pair : m_animationStates) {
            if (pair.second.isPlaying && pair.first != animationName) {
                currentAnimation = pair.first;
                break;
            }
        }
        
        if (!currentAnimation.empty()) {
            m_crossFadeState.fromAnimation = currentAnimation;
            m_crossFadeState.toAnimation = animationName;
            m_crossFadeState.fadeTime = fadeTime;
            m_crossFadeState.currentTime = 0.0f;
            m_crossFadeState.active = true;
            
            state->isPlaying = true;
            state->time = 0.0f;
            
            Logger::Debug("Cross-fading from " + currentAnimation + " to " + animationName + 
                         " over " + std::to_string(fadeTime) + " seconds");
        } else {
            Play(animationName, 0.0f);
        }
    }
    
    void Animator::Blend(const std::string& animationName, float targetWeight, float fadeTime) {
        (void)fadeTime;
        auto* state = GetAnimationState(animationName);
        if (!state) {
            Logger::Error("Animation not found for blend: " + animationName);
            return;
        }
        
        state->weight = std::max(0.0f, std::min(1.0f, targetWeight));
        state->isPlaying = true;
        
        Logger::Debug("Blending animation " + animationName + " to weight " + std::to_string(targetWeight));
    }
    
    void Animator::Update(float deltaTime) {
        if (!m_target) return;
        
        ProcessCrossFade(deltaTime);
        
        for (auto& pair : m_animationStates) {
            if (pair.second.isPlaying && pair.second.enabled) {
                UpdateAnimationState(pair.second, deltaTime);
                ApplyAnimation(pair.second);
            }
        }
        
        if (m_eventCallback) {
            for (const auto& pair : m_animationStates) {
                if (pair.second.isPlaying && pair.second.clip) {
                    float normalizedTime = GetNormalizedTime(pair.first);
                    if (normalizedTime >= 1.0f && !pair.second.clip->IsLooping()) {
                        m_eventCallback(pair.first + "_completed");
                    }
                }
            }
        }
    }
    
    void Animator::UpdateAnimationState(AnimationState& state, float deltaTime) {
        if (!state.clip) return;
        
        state.time += deltaTime * state.speed;
        
        float clipLength = state.clip->GetLength();
        if (state.time >= clipLength) {
            switch (state.clip->GetWrapMode()) {
                case AnimationWrapMode::Once:
                    state.time = clipLength;
                    state.isPlaying = false;
                    break;
                case AnimationWrapMode::Loop:
                    state.time = std::fmod(state.time, clipLength);
                    break;
                case AnimationWrapMode::PingPong:
                    state.time = std::fmod(state.time, clipLength);
                    break;
                case AnimationWrapMode::ClampForever:
                    state.time = clipLength;
                    break;
            }
        }
    }
    
    void Animator::ApplyAnimation(const AnimationState& state) {
        if (!state.clip || !m_target) return;
        
        state.clip->Sample(state.time, m_target);
    }
    
    void Animator::ProcessCrossFade(float deltaTime) {
        if (!m_crossFadeState.active) return;
        
        m_crossFadeState.currentTime += deltaTime;
        float t = m_crossFadeState.currentTime / m_crossFadeState.fadeTime;
        
        if (t >= 1.0f) {
            auto* fromState = GetAnimationState(m_crossFadeState.fromAnimation);
            auto* toState = GetAnimationState(m_crossFadeState.toAnimation);
            
            if (fromState) {
                fromState->isPlaying = false;
                fromState->weight = 0.0f;
            }
            
            if (toState) {
                toState->weight = 1.0f;
            }
            
            m_crossFadeState.active = false;
            Logger::Debug("Cross-fade completed");
        } else {
            auto* fromState = GetAnimationState(m_crossFadeState.fromAnimation);
            auto* toState = GetAnimationState(m_crossFadeState.toAnimation);
            
            if (fromState) {
                fromState->weight = 1.0f - t;
            }
            
            if (toState) {
                toState->weight = t;
            }
        }
    }
    
    AnimationState* Animator::GetAnimationState(const std::string& name) {
        auto it = m_animationStates.find(name);
        return (it != m_animationStates.end()) ? &it->second : nullptr;
    }
    
    const AnimationState* Animator::GetAnimationState(const std::string& name) const {
        auto it = m_animationStates.find(name);
        return (it != m_animationStates.end()) ? &it->second : nullptr;
    }
}
