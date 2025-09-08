#include "AudioSource.h"
#include "AudioClip.h"
#include "../Core/Logging/Logger.h"
#ifdef OPENAL_AVAILABLE
#include <AL/al.h>
#endif

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
        
#ifdef OPENAL_AVAILABLE
        alGenSources(1, &m_sourceID);
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            Logger::Error("Failed to generate OpenAL source: " + std::to_string(error));
            return false;
        }
#endif
        
        UpdateOpenALProperties();
        
        m_initialized = true;
        return true;
    }
    
    void AudioSource::Shutdown() {
        if (!m_initialized) return;
        
        Stop();
        
#ifdef OPENAL_AVAILABLE
        if (m_sourceID != 0) {
            alDeleteSources(1, &m_sourceID);
            m_sourceID = 0;
        }
#else
        if (m_sourceID != 0) {
            m_sourceID = 0;
        }
#endif
        
        m_initialized = false;
    }
    
    void AudioSource::Play() {
        if (!m_initialized || !m_clip) {
            Logger::Warning("AudioSource not initialized or no clip assigned");
            return;
        }
        
#ifdef OPENAL_AVAILABLE
        alSourcePlay(m_sourceID);
#endif
        
        Logger::Debug("Playing audio source");
    }
    
    void AudioSource::Pause() {
        if (!m_initialized) {
            Logger::Warning("AudioSource not initialized");
            return;
        }
        
#ifdef OPENAL_AVAILABLE
        alSourcePause(m_sourceID);
#endif
        
        Logger::Debug("Paused audio source");
    }
    
    void AudioSource::Stop() {
        if (!m_initialized) {
            Logger::Warning("AudioSource not initialized");
            return;
        }
        
#ifdef OPENAL_AVAILABLE
        alSourceStop(m_sourceID);
#endif
        
        Logger::Debug("Stopped audio source");
    }
    
    void AudioSource::SetClip(AudioClip* clip) {
        m_clip = clip;
        
        if (m_initialized && m_clip) {
#ifdef OPENAL_AVAILABLE
            alSourcei(m_sourceID, AL_BUFFER, m_clip->GetBufferID());
#endif
        }
    }
    
    void AudioSource::SetVolume(float volume) {
        m_volume = std::max(0.0f, std::min(1.0f, volume));
        
        if (m_initialized) {
            UpdateOpenALProperties();
        }
    }
    
    void AudioSource::SetPitch(float pitch) {
        m_pitch = std::max(0.1f, std::min(3.0f, pitch));
        
        if (m_initialized) {
            UpdateOpenALProperties();
        }
    }
    
    void AudioSource::SetLooping(bool looping) {
        m_looping = looping;
        
        if (m_initialized) {
            UpdateOpenALProperties();
        }
    }
    
    void AudioSource::SetPosition(const Vector3& position) {
        m_position = position;
        
        if (m_initialized) {
            UpdateOpenALProperties();
        }
    }
    
    void AudioSource::SetVelocity(const Vector3& velocity) {
        m_velocity = velocity;
        
        if (m_initialized) {
            UpdateOpenALProperties();
        }
    }
    
    void AudioSource::SetMinDistance(float distance) {
        m_minDistance = std::max(0.0f, distance);
        
        if (m_initialized) {
            UpdateOpenALProperties();
        }
    }
    
    void AudioSource::SetMaxDistance(float distance) {
        m_maxDistance = std::max(m_minDistance, distance);
        
        if (m_initialized) {
            UpdateOpenALProperties();
        }
    }
    
    void AudioSource::SetRolloffFactor(float rolloff) {
        m_rolloffFactor = std::max(0.0f, rolloff);
        
        if (m_initialized) {
            UpdateOpenALProperties();
        }
    }
    
    AudioSourceState AudioSource::GetState() const {
        if (!m_initialized) {
            return AudioSourceState::Stopped;
        }
        
#ifdef OPENAL_AVAILABLE
        ALint state;
        alGetSourcei(m_sourceID, AL_SOURCE_STATE, &state);
        
        switch (state) {
            case AL_PLAYING:
                return AudioSourceState::Playing;
            case AL_PAUSED:
                return AudioSourceState::Paused;
            case AL_STOPPED:
            case AL_INITIAL:
            default:
                return AudioSourceState::Stopped;
        }
#else
        return AudioSourceState::Stopped;
#endif
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
        
#ifdef OPENAL_AVAILABLE
        ALfloat offset;
        alGetSourcef(m_sourceID, AL_SEC_OFFSET, &offset);
        return offset;
#else
        return 0.0f;
#endif
    }
    
    void AudioSource::SetPlaybackPosition(float seconds) {
        if (!m_initialized) {
            return;
        }
        
#ifdef OPENAL_AVAILABLE
        alSourcef(m_sourceID, AL_SEC_OFFSET, seconds);
#else
        (void)seconds;
#endif
    }
    
    void AudioSource::UpdateOpenALProperties() {
        if (!m_initialized) return;
        
#ifdef OPENAL_AVAILABLE
        alSourcef(m_sourceID, AL_GAIN, m_volume);
        alSourcef(m_sourceID, AL_PITCH, m_pitch);
        alSourcei(m_sourceID, AL_LOOPING, m_looping ? AL_TRUE : AL_FALSE);
        alSource3f(m_sourceID, AL_POSITION, m_position.x, m_position.y, m_position.z);
        alSource3f(m_sourceID, AL_VELOCITY, m_velocity.x, m_velocity.y, m_velocity.z);
        alSourcef(m_sourceID, AL_REFERENCE_DISTANCE, m_minDistance);
        alSourcef(m_sourceID, AL_MAX_DISTANCE, m_maxDistance);
        alSourcef(m_sourceID, AL_ROLLOFF_FACTOR, m_rolloffFactor);
#endif
    }
}
