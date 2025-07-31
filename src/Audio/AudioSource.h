#pragma once

#include "../Core/Math/Vector3.h"

namespace GameEngine {
    class AudioClip;
    
    enum class AudioSourceState {
        Stopped,
        Playing,
        Paused
    };
    
    class AudioSource {
    public:
        AudioSource();
        ~AudioSource();
        
        bool Initialize();
        void Shutdown();
        
        // Playback control
        void Play();
        void Pause();
        void Stop();
        
        // Audio clip assignment
        void SetClip(AudioClip* clip);
        AudioClip* GetClip() const { return m_clip; }
        
        // Audio properties
        void SetVolume(float volume);
        float GetVolume() const { return m_volume; }
        
        void SetPitch(float pitch);
        float GetPitch() const { return m_pitch; }
        
        void SetLooping(bool looping);
        bool IsLooping() const { return m_looping; }
        
        // 3D audio properties
        void SetPosition(const Vector3& position);
        Vector3 GetPosition() const { return m_position; }
        
        void SetVelocity(const Vector3& velocity);
        Vector3 GetVelocity() const { return m_velocity; }
        
        void SetMinDistance(float distance);
        float GetMinDistance() const { return m_minDistance; }
        
        void SetMaxDistance(float distance);
        float GetMaxDistance() const { return m_maxDistance; }
        
        void SetRolloffFactor(float rolloff);
        float GetRolloffFactor() const { return m_rolloffFactor; }
        
        // State queries
        AudioSourceState GetState() const;
        bool IsPlaying() const;
        bool IsPaused() const;
        bool IsStopped() const;
        
        // Playback position
        float GetPlaybackPosition() const;
        void SetPlaybackPosition(float seconds);
        
        // OpenAL source ID
        unsigned int GetSourceID() const { return m_sourceID; }
        
    private:
        void UpdateOpenALProperties();
        
        AudioClip* m_clip = nullptr;
        
        // Audio properties
        float m_volume = 1.0f;
        float m_pitch = 1.0f;
        bool m_looping = false;
        
        // 3D audio properties
        Vector3 m_position = Vector3::Zero;
        Vector3 m_velocity = Vector3::Zero;
        float m_minDistance = 1.0f;
        float m_maxDistance = 100.0f;
        float m_rolloffFactor = 1.0f;
        
        unsigned int m_sourceID = 0;
        bool m_initialized = false;
    };
}
