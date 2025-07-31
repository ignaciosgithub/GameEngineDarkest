#include "AudioSource.h"
#include "AudioClip.h"
#include "../Core/Logging/Logger.h"

namespace GameEngine {
    
    AudioSource::AudioSource() = default;
    
    AudioSource::~AudioSource() {
        Shutdown();
    }
    
    bool AudioSource::Initialize() {
        if (m_initialized) {
            Logger::Warning("AudioSource already initialized");
            return true;
        }
        
        // }
        
        UpdateOpenALProperties();
        
        m_initialized = true;
        return true;
    }
    
    void AudioSource::Shutdown() {
        if (!m_initialized) return;
        
        Stop();
        
        if (m_sourceID != 0) {
            m_sourceID = 0;
        }
        
        m_initialized = false;
    }
    
    void AudioSource::Play() {
        if (!m_initialized || !m_clip) {
            Logger::Warning("AudioSource not initialized or no clip assigned");
            return;
        }
        
        
        Logger::Debug("Playing audio source");
    }
    
    void AudioSource::Pause() {
        if (!m_initialized) {
            Logger::Warning("AudioSource not initialized");
            return;
        }
        
        
        Logger::Debug("Paused audio source");
    }
    
    void AudioSource::Stop() {
        if (!m_initialized) {
            Logger::Warning("AudioSource not initialized");
            return;
        }
        
        
        Logger::Debug("Stopped audio source");
    }
    
    void AudioSource::SetClip(AudioClip* clip) {
        m_clip = clip;
        
        if (m_initialized && m_clip) {
        }
    }
    
    void AudioSource::SetVolume(float volume) {
        m_volume = std::max(0.0f, std::min(1.0f, volume));
        
        if (m_initialized) {
        }
    }
    
    void AudioSource::SetPitch(float pitch) {
        m_pitch = std::max(0.1f, std::min(3.0f, pitch));
        
        if (m_initialized) {
        }
    }
    
    void AudioSource::SetLooping(bool looping) {
        m_looping = looping;
        
        if (m_initialized) {
        }
    }
    
    void AudioSource::SetPosition(const Vector3& position) {
        m_position = position;
        
        if (m_initialized) {
        }
    }
    
    void AudioSource::SetVelocity(const Vector3& velocity) {
        m_velocity = velocity;
        
        if (m_initialized) {
        }
    }
    
    void AudioSource::SetMinDistance(float distance) {
        m_minDistance = std::max(0.0f, distance);
        
        if (m_initialized) {
        }
    }
    
    void AudioSource::SetMaxDistance(float distance) {
        m_maxDistance = std::max(m_minDistance, distance);
        
        if (m_initialized) {
        }
    }
    
    void AudioSource::SetRolloffFactor(float rolloff) {
        m_rolloffFactor = std::max(0.0f, rolloff);
        
        if (m_initialized) {
        }
    }
    
    AudioSourceState AudioSource::GetState() const {
        if (!m_initialized) {
            return AudioSourceState::Stopped;
        }
        
        // 
        // }
        
        return AudioSourceState::Stopped;
    }
    
    bool AudioSource::IsPlaying() const {
        return GetState() == AudioSourceState::Playing;
    }
    
    bool AudioSource::IsPaused() const {
        return GetState() == AudioSourceState::Paused;
    }
    
    bool AudioSource::IsStopped() const {
        return GetState() == AudioSourceState::Stopped;
    }
    
    float AudioSource::GetPlaybackPosition() const {
        if (!m_initialized) {
            return 0.0f;
        }
        
        
        return 0.0f;
    }
    
    void AudioSource::SetPlaybackPosition(float seconds) {
        if (!m_initialized) {
            return;
        }
        
        (void)seconds;
        
    }
    
    void AudioSource::UpdateOpenALProperties() {
        if (!m_initialized) return;
        
    }
}
