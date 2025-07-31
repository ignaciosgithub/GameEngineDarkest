#pragma once

#include "../ECS/Component.h"
#include "../../Audio/AudioSource.h"
#include "../../Audio/AudioClip.h"
#include "../Math/Vector3.h"
#include <memory>
#include <string>

namespace GameEngine {
    
    class AudioComponent : public Component<AudioComponent> {
    public:
        AudioComponent();
        ~AudioComponent();
        
        // Audio source management
        void SetAudioSource(std::shared_ptr<AudioSource> source);
        std::shared_ptr<AudioSource> GetAudioSource() const { return m_audioSource; }
        
        // Audio clip management
        void SetAudioClip(AudioClip* clip);
        AudioClip* GetAudioClip() const;
        
        // Playback control
        void Play();
        void Pause();
        void Stop();
        
        // Audio properties
        void SetVolume(float volume);
        float GetVolume() const;
        
        void SetPitch(float pitch);
        float GetPitch() const;
        
        void SetLooping(bool looping);
        bool IsLooping() const;
        
        // 3D audio properties
        void SetPosition(const Vector3& position);
        Vector3 GetPosition() const;
        
        void SetVelocity(const Vector3& velocity);
        Vector3 GetVelocity() const;
        
        void SetMinDistance(float distance);
        float GetMinDistance() const;
        
        void SetMaxDistance(float distance);
        float GetMaxDistance() const;
        
        void SetRolloffFactor(float rolloff);
        float GetRolloffFactor() const;
        
        // State queries
        bool IsPlaying() const;
        bool IsPaused() const;
        bool IsStopped() const;
        
        // Auto-play settings
        void SetPlayOnAwake(bool playOnAwake) { m_playOnAwake = playOnAwake; }
        bool GetPlayOnAwake() const { return m_playOnAwake; }
        
        // Spatial audio settings
        void SetSpatial(bool spatial) { m_spatial = spatial; }
        bool IsSpatial() const { return m_spatial; }
        
        // Update method for component system
        void Update(float deltaTime);
        
    private:
        std::shared_ptr<AudioSource> m_audioSource;
        bool m_playOnAwake = false;
        bool m_spatial = true;
        bool m_initialized = false;
        
        void EnsureAudioSourceInitialized();
    };
}
