#pragma once

#include "../Core/Math/Vector3.h"
#include "../Core/Math/Quaternion.h"
#include "../Core/Math/Transform.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace GameEngine {
    class GameObject;
    class AnimationClip;
    class Animator;
    
    enum class AnimationBlendMode {
        Override,
        Additive,
        Multiply
    };
    
    enum class AnimationWrapMode {
        Once,
        Loop,
        PingPong,
        ClampForever
    };
    
    struct Keyframe {
        float time;
        Vector3 value;
        Vector3 inTangent;
        Vector3 outTangent;
        
        Keyframe() : time(0.0f), value(Vector3::Zero), inTangent(Vector3::Zero), outTangent(Vector3::Zero) {}
        Keyframe(float t, const Vector3& v) : time(t), value(v), inTangent(Vector3::Zero), outTangent(Vector3::Zero) {}
    };
    
    struct RotationKeyframe {
        float time;
        Quaternion rotation;
        
        RotationKeyframe() : time(0.0f), rotation(Quaternion()) {}
        RotationKeyframe(float t, const Quaternion& r) : time(t), rotation(r) {}
    };
    
    class AnimationCurve {
    public:
        AnimationCurve();
        ~AnimationCurve();
        
        void AddKeyframe(const Keyframe& keyframe);
        void AddRotationKeyframe(const RotationKeyframe& keyframe);
        
        Vector3 EvaluatePosition(float time) const;
        Quaternion EvaluateRotation(float time) const;
        
        void SetPreWrapMode(AnimationWrapMode mode) { m_preWrapMode = mode; }
        void SetPostWrapMode(AnimationWrapMode mode) { m_postWrapMode = mode; }
        
        float GetLength() const;
        void Clear();
        
    private:
        std::vector<Keyframe> m_positionKeys;
        std::vector<RotationKeyframe> m_rotationKeys;
        AnimationWrapMode m_preWrapMode = AnimationWrapMode::ClampForever;
        AnimationWrapMode m_postWrapMode = AnimationWrapMode::ClampForever;
        
        float WrapTime(float time, float length) const;
        Vector3 InterpolatePosition(const Keyframe& a, const Keyframe& b, float t) const;
        Quaternion InterpolateRotation(const RotationKeyframe& a, const RotationKeyframe& b, float t) const;
    };
    
    class AnimationClip {
    public:
        AnimationClip(const std::string& name);
        ~AnimationClip();
        
        const std::string& GetName() const { return m_name; }
        float GetLength() const { return m_length; }
        
        void SetLength(float length) { m_length = length; }
        void SetWrapMode(AnimationWrapMode mode) { m_wrapMode = mode; }
        AnimationWrapMode GetWrapMode() const { return m_wrapMode; }
        
        void AddCurve(const std::string& propertyPath, std::shared_ptr<AnimationCurve> curve);
        std::shared_ptr<AnimationCurve> GetCurve(const std::string& propertyPath) const;
        
        void Sample(float time, GameObject* target) const;
        
        bool IsLooping() const { return m_wrapMode == AnimationWrapMode::Loop || m_wrapMode == AnimationWrapMode::PingPong; }
        
    private:
        std::string m_name;
        float m_length = 1.0f;
        AnimationWrapMode m_wrapMode = AnimationWrapMode::Loop;
        std::unordered_map<std::string, std::shared_ptr<AnimationCurve>> m_curves;
    };
    
    struct AnimationState {
        std::shared_ptr<AnimationClip> clip;
        float time = 0.0f;
        float speed = 1.0f;
        float weight = 1.0f;
        bool enabled = true;
        bool isPlaying = false;
        AnimationBlendMode blendMode = AnimationBlendMode::Override;
        
        AnimationState() = default;
        AnimationState(std::shared_ptr<AnimationClip> animClip) : clip(animClip) {}
    };
    
    class AnimationSystem {
    public:
        AnimationSystem();
        ~AnimationSystem();
        
        bool Initialize();
        void Update(float deltaTime);
        void Shutdown();
        
        // Animation clip management
        void RegisterAnimationClip(std::shared_ptr<AnimationClip> clip);
        std::shared_ptr<AnimationClip> GetAnimationClip(const std::string& name) const;
        void UnregisterAnimationClip(const std::string& name);
        
        // Animator management
        void RegisterAnimator(std::shared_ptr<Animator> animator);
        void UnregisterAnimator(std::shared_ptr<Animator> animator);
        
        // Global animation controls
        void SetGlobalTimeScale(float timeScale) { m_globalTimeScale = timeScale; }
        float GetGlobalTimeScale() const { return m_globalTimeScale; }
        
        void PauseAll();
        void ResumeAll();
        
        bool IsInitialized() const { return m_initialized; }
        
    private:
        bool m_initialized = false;
        float m_globalTimeScale = 1.0f;
        bool m_globalPaused = false;
        
        std::unordered_map<std::string, std::shared_ptr<AnimationClip>> m_animationClips;
        std::vector<std::shared_ptr<Animator>> m_animators;
        
        void UpdateAnimators(float deltaTime);
    };
}
