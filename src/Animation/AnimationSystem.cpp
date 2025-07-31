#include "AnimationSystem.h"
#include "Animator.h"
#include "../Core/GameObject/GameObject.h"
#include "../Core/Logging/Logger.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {
    
    AnimationCurve::AnimationCurve() = default;
    AnimationCurve::~AnimationCurve() = default;
    
    void AnimationCurve::AddKeyframe(const Keyframe& keyframe) {
        auto it = std::lower_bound(m_positionKeys.begin(), m_positionKeys.end(), keyframe,
            [](const Keyframe& a, const Keyframe& b) { return a.time < b.time; });
        m_positionKeys.insert(it, keyframe);
    }
    
    void AnimationCurve::AddRotationKeyframe(const RotationKeyframe& keyframe) {
        auto it = std::lower_bound(m_rotationKeys.begin(), m_rotationKeys.end(), keyframe,
            [](const RotationKeyframe& a, const RotationKeyframe& b) { return a.time < b.time; });
        m_rotationKeys.insert(it, keyframe);
    }
    
    Vector3 AnimationCurve::EvaluatePosition(float time) const {
        if (m_positionKeys.empty()) {
            return Vector3::Zero;
        }
        
        if (m_positionKeys.size() == 1) {
            return m_positionKeys[0].value;
        }
        
        float length = GetLength();
        float wrappedTime = WrapTime(time, length);
        
        auto it = std::lower_bound(m_positionKeys.begin(), m_positionKeys.end(), wrappedTime,
            [](const Keyframe& keyframe, float t) { return keyframe.time < t; });
        
        if (it == m_positionKeys.begin()) {
            return m_positionKeys[0].value;
        }
        
        if (it == m_positionKeys.end()) {
            return m_positionKeys.back().value;
        }
        
        const Keyframe& keyB = *it;
        const Keyframe& keyA = *(it - 1);
        
        float t = (wrappedTime - keyA.time) / (keyB.time - keyA.time);
        return InterpolatePosition(keyA, keyB, t);
    }
    
    Quaternion AnimationCurve::EvaluateRotation(float time) const {
        if (m_rotationKeys.empty()) {
            return Quaternion();
        }
        
        if (m_rotationKeys.size() == 1) {
            return m_rotationKeys[0].rotation;
        }
        
        float length = GetLength();
        float wrappedTime = WrapTime(time, length);
        
        auto it = std::lower_bound(m_rotationKeys.begin(), m_rotationKeys.end(), wrappedTime,
            [](const RotationKeyframe& keyframe, float t) { return keyframe.time < t; });
        
        if (it == m_rotationKeys.begin()) {
            return m_rotationKeys[0].rotation;
        }
        
        if (it == m_rotationKeys.end()) {
            return m_rotationKeys.back().rotation;
        }
        
        const RotationKeyframe& keyB = *it;
        const RotationKeyframe& keyA = *(it - 1);
        
        float t = (wrappedTime - keyA.time) / (keyB.time - keyA.time);
        return InterpolateRotation(keyA, keyB, t);
    }
    
    float AnimationCurve::GetLength() const {
        float maxTime = 0.0f;
        
        if (!m_positionKeys.empty()) {
            maxTime = std::max(maxTime, m_positionKeys.back().time);
        }
        
        if (!m_rotationKeys.empty()) {
            maxTime = std::max(maxTime, m_rotationKeys.back().time);
        }
        
        return maxTime;
    }
    
    void AnimationCurve::Clear() {
        m_positionKeys.clear();
        m_rotationKeys.clear();
    }
    
    float AnimationCurve::WrapTime(float time, float length) const {
        if (length <= 0.0f) return 0.0f;
        
        if (time < 0.0f) {
            switch (m_preWrapMode) {
                case AnimationWrapMode::Loop:
                    return length + std::fmod(time, length);
                case AnimationWrapMode::PingPong:
                    return std::abs(std::fmod(time, length * 2.0f));
                case AnimationWrapMode::ClampForever:
                default:
                    return 0.0f;
            }
        } else if (time > length) {
            switch (m_postWrapMode) {
                case AnimationWrapMode::Loop:
                    return std::fmod(time, length);
                case AnimationWrapMode::PingPong: {
                    float cycle = std::fmod(time, length * 2.0f);
                    return cycle > length ? length * 2.0f - cycle : cycle;
                }
                case AnimationWrapMode::ClampForever:
                default:
                    return length;
            }
        }
        
        return time;
    }
    
    Vector3 AnimationCurve::InterpolatePosition(const Keyframe& a, const Keyframe& b, float t) const {
        return Vector3::Lerp(a.value, b.value, t);
    }
    
    Quaternion AnimationCurve::InterpolateRotation(const RotationKeyframe& a, const RotationKeyframe& b, float t) const {
        return Quaternion::Slerp(a.rotation, b.rotation, t);
    }
    
    AnimationClip::AnimationClip(const std::string& name) : m_name(name) {
    }
    
    AnimationClip::~AnimationClip() = default;
    
    void AnimationClip::AddCurve(const std::string& propertyPath, std::shared_ptr<AnimationCurve> curve) {
        m_curves[propertyPath] = curve;
        
        float curveLength = curve->GetLength();
        if (curveLength > m_length) {
            m_length = curveLength;
        }
    }
    
    std::shared_ptr<AnimationCurve> AnimationClip::GetCurve(const std::string& propertyPath) const {
        auto it = m_curves.find(propertyPath);
        return (it != m_curves.end()) ? it->second : nullptr;
    }
    
    void AnimationClip::Sample(float time, GameObject* target) const {
        if (!target) return;
        
        auto positionCurve = GetCurve("transform.position");
        if (positionCurve) {
            Vector3 position = positionCurve->EvaluatePosition(time);
            target->GetTransform()->transform.SetPosition(position);
        }
        
        auto rotationCurve = GetCurve("transform.rotation");
        if (rotationCurve) {
            Quaternion rotation = rotationCurve->EvaluateRotation(time);
            target->GetTransform()->transform.SetRotation(rotation);
        }
        
        auto scaleCurve = GetCurve("transform.scale");
        if (scaleCurve) {
            Vector3 scale = scaleCurve->EvaluatePosition(time);
            target->GetTransform()->transform.SetScale(scale);
        }
    }
    
    AnimationSystem::AnimationSystem() = default;
    AnimationSystem::~AnimationSystem() {
        Shutdown();
    }
    
    bool AnimationSystem::Initialize() {
        if (m_initialized) {
            Logger::Warning("AnimationSystem already initialized");
            return true;
        }
        
        m_initialized = true;
        Logger::Info("AnimationSystem initialized successfully");
        return true;
    }
    
    void AnimationSystem::Update(float deltaTime) {
        if (!m_initialized || m_globalPaused) return;
        
        float scaledDeltaTime = deltaTime * m_globalTimeScale;
        UpdateAnimators(scaledDeltaTime);
    }
    
    void AnimationSystem::Shutdown() {
        if (!m_initialized) return;
        
        m_animators.clear();
        m_animationClips.clear();
        
        m_initialized = false;
        Logger::Info("AnimationSystem shutdown successfully");
    }
    
    void AnimationSystem::RegisterAnimationClip(std::shared_ptr<AnimationClip> clip) {
        if (!clip) {
            Logger::Error("Cannot register null animation clip");
            return;
        }
        
        m_animationClips[clip->GetName()] = clip;
        Logger::Debug("Registered animation clip: " + clip->GetName());
    }
    
    std::shared_ptr<AnimationClip> AnimationSystem::GetAnimationClip(const std::string& name) const {
        auto it = m_animationClips.find(name);
        return (it != m_animationClips.end()) ? it->second : nullptr;
    }
    
    void AnimationSystem::UnregisterAnimationClip(const std::string& name) {
        auto it = m_animationClips.find(name);
        if (it != m_animationClips.end()) {
            m_animationClips.erase(it);
            Logger::Debug("Unregistered animation clip: " + name);
        }
    }
    
    void AnimationSystem::RegisterAnimator(std::shared_ptr<Animator> animator) {
        if (!animator) {
            Logger::Error("Cannot register null animator");
            return;
        }
        
        m_animators.push_back(animator);
        Logger::Debug("Registered animator");
    }
    
    void AnimationSystem::UnregisterAnimator(std::shared_ptr<Animator> animator) {
        auto it = std::find(m_animators.begin(), m_animators.end(), animator);
        if (it != m_animators.end()) {
            m_animators.erase(it);
            Logger::Debug("Unregistered animator");
        }
    }
    
    void AnimationSystem::PauseAll() {
        m_globalPaused = true;
        Logger::Debug("All animations paused");
    }
    
    void AnimationSystem::ResumeAll() {
        m_globalPaused = false;
        Logger::Debug("All animations resumed");
    }
    
    void AnimationSystem::UpdateAnimators(float deltaTime) {
        for (auto& animator : m_animators) {
            if (animator) {
                animator->Update(deltaTime);
            }
        }
    }
}
