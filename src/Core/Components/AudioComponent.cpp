#include "AudioComponent.h"
#include "../Logging/Logger.h"

namespace GameEngine {
    
    AudioComponent::AudioComponent() = default;
    
    AudioComponent::~AudioComponent() = default;
    
    void AudioComponent::SetAudioSource(std::shared_ptr<AudioSource> source) {
        m_audioSource = source;
        m_initialized = (m_audioSource != nullptr);
    }
    
    void AudioComponent::SetAudioClip(AudioClip* clip) {
        EnsureAudioSourceInitialized();
        if (m_audioSource) {
            m_audioSource->SetClip(clip);
        }
    }
    
    AudioClip* AudioComponent::GetAudioClip() const {
        if (m_audioSource) {
            return m_audioSource->GetClip();
        }
        return nullptr;
    }
    
    void AudioComponent::Play() {
        EnsureAudioSourceInitialized();
        if (m_audioSource) {
            m_audioSource->Play();
            Logger::Debug("AudioComponent: Playing audio");
        }
    }
    
    void AudioComponent::Pause() {
        if (m_audioSource) {
            m_audioSource->Pause();
            Logger::Debug("AudioComponent: Paused audio");
        }
    }
    
    void AudioComponent::Stop() {
        if (m_audioSource) {
            m_audioSource->Stop();
            Logger::Debug("AudioComponent: Stopped audio");
        }
    }
    
    void AudioComponent::SetVolume(float volume) {
        EnsureAudioSourceInitialized();
        if (m_audioSource) {
            m_audioSource->SetVolume(volume);
        }
    }
    
    float AudioComponent::GetVolume() const {
        if (m_audioSource) {
            return m_audioSource->GetVolume();
        }
        return 0.0f;
    }
    
    void AudioComponent::SetPitch(float pitch) {
        EnsureAudioSourceInitialized();
        if (m_audioSource) {
            m_audioSource->SetPitch(pitch);
        }
    }
    
    float AudioComponent::GetPitch() const {
        if (m_audioSource) {
            return m_audioSource->GetPitch();
        }
        return 1.0f;
    }
    
    void AudioComponent::SetLooping(bool looping) {
        EnsureAudioSourceInitialized();
        if (m_audioSource) {
            m_audioSource->SetLooping(looping);
        }
    }
    
    bool AudioComponent::IsLooping() const {
        if (m_audioSource) {
            return m_audioSource->IsLooping();
        }
        return false;
    }
    
    void AudioComponent::SetPosition(const Vector3& position) {
        EnsureAudioSourceInitialized();
        if (m_audioSource && m_spatial) {
            m_audioSource->SetPosition(position);
        }
    }
    
    Vector3 AudioComponent::GetPosition() const {
        if (m_audioSource) {
            return m_audioSource->GetPosition();
        }
        return Vector3::Zero;
    }
    
    void AudioComponent::SetVelocity(const Vector3& velocity) {
        EnsureAudioSourceInitialized();
        if (m_audioSource && m_spatial) {
            m_audioSource->SetVelocity(velocity);
        }
    }
    
    Vector3 AudioComponent::GetVelocity() const {
        if (m_audioSource) {
            return m_audioSource->GetVelocity();
        }
        return Vector3::Zero;
    }
    
    void AudioComponent::SetMinDistance(float distance) {
        EnsureAudioSourceInitialized();
        if (m_audioSource && m_spatial) {
            m_audioSource->SetMinDistance(distance);
        }
    }
    
    float AudioComponent::GetMinDistance() const {
        if (m_audioSource) {
            return m_audioSource->GetMinDistance();
        }
        return 1.0f;
    }
    
    void AudioComponent::SetMaxDistance(float distance) {
        EnsureAudioSourceInitialized();
        if (m_audioSource && m_spatial) {
            m_audioSource->SetMaxDistance(distance);
        }
    }
    
    float AudioComponent::GetMaxDistance() const {
        if (m_audioSource) {
            return m_audioSource->GetMaxDistance();
        }
        return 100.0f;
    }
    
    void AudioComponent::SetRolloffFactor(float rolloff) {
        EnsureAudioSourceInitialized();
        if (m_audioSource && m_spatial) {
            m_audioSource->SetRolloffFactor(rolloff);
        }
    }
    
    float AudioComponent::GetRolloffFactor() const {
        if (m_audioSource) {
            return m_audioSource->GetRolloffFactor();
        }
        return 1.0f;
    }
    
    bool AudioComponent::IsPlaying() const {
        if (m_audioSource) {
            return m_audioSource->IsPlaying();
        }
        return false;
    }
    
    bool AudioComponent::IsPaused() const {
        if (m_audioSource) {
            return m_audioSource->IsPaused();
        }
        return false;
    }
    
    bool AudioComponent::IsStopped() const {
        if (m_audioSource) {
            return m_audioSource->IsStopped();
        }
        return true;
    }
    
    void AudioComponent::Update(float deltaTime) {
        (void)deltaTime;
        
        if (m_playOnAwake && !m_initialized && m_audioSource) {
            Play();
            m_playOnAwake = false; // Only play once
        }
    }
    
    void AudioComponent::EnsureAudioSourceInitialized() {
        if (!m_audioSource) {
            m_audioSource = std::make_shared<AudioSource>();
            if (m_audioSource->Initialize()) {
                m_initialized = true;
                Logger::Debug("AudioComponent: Created and initialized AudioSource");
            } else {
                Logger::Error("AudioComponent: Failed to initialize AudioSource");
                m_audioSource.reset();
            }
        }
    }
}
