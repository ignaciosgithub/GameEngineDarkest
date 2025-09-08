#include "AudioManager.h"
#include "AudioClip.h"
#include "AudioSource.h"
#include "../Core/Logging/Logger.h"
#include "../Core/Profiling/Profiler.h"
#ifdef OPENAL_AVAILABLE
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <algorithm>

namespace GameEngine {
    
    AudioManager::AudioManager() = default;
    
    AudioManager::~AudioManager() {
        Shutdown();
    }
    
    bool AudioManager::Initialize() {
        if (m_initialized) {
            Logger::Warning("AudioManager already initialized");
            return true;
        }
        
#ifdef OPENAL_AVAILABLE
        m_alDevice = alcOpenDevice(nullptr);
        if (!m_alDevice) {
            Logger::Error("Failed to open OpenAL device");
            return false;
        }
        
        m_alContext = alcCreateContext(static_cast<ALCdevice*>(m_alDevice), nullptr);
        if (!m_alContext) {
            Logger::Error("Failed to create OpenAL context");
            alcCloseDevice(static_cast<ALCdevice*>(m_alDevice));
            m_alDevice = nullptr;
            return false;
        }
        
        alcMakeContextCurrent(static_cast<ALCcontext*>(m_alContext));
        
        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        float orientation[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
        alListenerfv(AL_ORIENTATION, orientation);
        
        Logger::Info("AudioManager initialized with OpenAL");
#else
        Logger::Info("AudioManager initialized (basic implementation - OpenAL not available)");
#endif
        
        m_initialized = true;
        return true;
    }
    
    void AudioManager::Update() {
        PROFILE_SCOPE("AudioManager::Update");
        if (!m_initialized) return;
        
        {
            PROFILE_SCOPE("Audio::CleanupFinishedSources");
            CleanupFinishedSources();
        }
        
    }
    
    void AudioManager::Shutdown() {
        if (!m_initialized) return;
        
        for (auto& source : m_audioSources) {
            if (source) {
                source->Stop();
                source->Shutdown();
            }
        }
        m_audioSources.clear();
        
        m_audioClips.clear();
        
#ifdef OPENAL_AVAILABLE
        if (m_alContext) {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(static_cast<ALCcontext*>(m_alContext));
            m_alContext = nullptr;
        }
        
        if (m_alDevice) {
            alcCloseDevice(static_cast<ALCdevice*>(m_alDevice));
            m_alDevice = nullptr;
        }
#else
        if (m_alContext) {
            m_alContext = nullptr;
        }
        
        if (m_alDevice) {
            m_alDevice = nullptr;
        }
#endif
        
        m_initialized = false;
        Logger::Info("AudioManager shutdown successfully");
    }
    
    void AudioManager::PlaySound(const std::string& filepath, float volume) {
        PROFILE_SCOPE("AudioManager::PlaySound");
        if (!m_initialized) {
            Logger::Warning("AudioManager not initialized");
            return;
        }
        
        AudioClip* clip = LoadAudioClip(filepath);
        if (clip) {
            PlaySound(clip, volume);
        }
    }
    
    void AudioManager::PlaySound(AudioClip* clip, float volume) {
        if (!m_initialized || !clip) {
            Logger::Warning("AudioManager not initialized or invalid clip");
            return;
        }
        
        AudioSource* source = CreateAudioSource();
        if (source) {
            source->SetClip(clip);
            source->SetVolume(volume * m_masterVolume);
            source->SetLooping(false);
            source->Play();
            
            Logger::Debug("Playing sound: " + clip->GetFilepath());
        }
    }
    
    void AudioManager::PlaySound3D(const std::string& filepath, const Vector3& position, float volume) {
        PROFILE_SCOPE("AudioManager::PlaySound3D");
        if (!m_initialized) {
            Logger::Warning("AudioManager not initialized");
            return;
        }
        
        AudioClip* clip = LoadAudioClip(filepath);
        if (clip) {
            PlaySound3D(clip, position, volume);
        }
    }
    
    void AudioManager::PlaySound3D(AudioClip* clip, const Vector3& position, float volume) {
        if (!m_initialized || !clip) {
            Logger::Warning("AudioManager not initialized or invalid clip");
            return;
        }
        
        AudioSource* source = CreateAudioSource();
        if (source) {
            source->SetClip(clip);
            source->SetVolume(volume * m_masterVolume);
            source->SetPosition(position);
            source->SetLooping(false);
            source->Play();
            
            Logger::Debug("Playing 3D sound: " + clip->GetFilepath() + 
                         " at position (" + std::to_string(position.x) + ", " + 
                         std::to_string(position.y) + ", " + std::to_string(position.z) + ")");
        }
    }
    
    AudioClip* AudioManager::LoadAudioClip(const std::string& filepath) {
        if (!m_initialized) {
            Logger::Warning("AudioManager not initialized");
            return nullptr;
        }
        
        auto it = m_audioClips.find(filepath);
        if (it != m_audioClips.end()) {
            return it->second.get();
        }
        
        auto clip = std::make_unique<AudioClip>();
        if (clip->LoadFromFile(filepath)) {
            AudioClip* clipPtr = clip.get();
            m_audioClips[filepath] = std::move(clip);
            Logger::Info("Loaded audio clip: " + filepath);
            return clipPtr;
        } else {
            Logger::Error("Failed to load audio clip: " + filepath);
            return nullptr;
        }
    }
    
    void AudioManager::UnloadAudioClip(const std::string& filepath) {
        auto it = m_audioClips.find(filepath);
        if (it != m_audioClips.end()) {
            it->second->Unload();
            m_audioClips.erase(it);
            Logger::Info("Unloaded audio clip: " + filepath);
        }
    }
    
    AudioSource* AudioManager::CreateAudioSource() {
        if (!m_initialized) {
            Logger::Warning("AudioManager not initialized");
            return nullptr;
        }
        
        auto source = std::make_unique<AudioSource>();
        if (source->Initialize()) {
            AudioSource* sourcePtr = source.get();
            m_audioSources.push_back(std::move(source));
            return sourcePtr;
        } else {
            Logger::Error("Failed to create audio source");
            return nullptr;
        }
    }
    
    void AudioManager::DestroyAudioSource(AudioSource* source) {
        if (!source) return;
        
        auto it = std::find_if(m_audioSources.begin(), m_audioSources.end(),
            [source](const std::unique_ptr<AudioSource>& ptr) {
                return ptr.get() == source;
            });
        
        if (it != m_audioSources.end()) {
            (*it)->Stop();
            (*it)->Shutdown();
            m_audioSources.erase(it);
        }
    }
    
    void AudioManager::SetMasterVolume(float volume) {
        m_masterVolume = std::max(0.0f, std::min(1.0f, volume));
        Logger::Debug("Master volume set to: " + std::to_string(m_masterVolume));
    }
    
    void AudioManager::SetListenerPosition(const Vector3& position) {
        m_listenerPosition = position;
        
    }
    
    void AudioManager::SetListenerOrientation(const Vector3& forward, const Vector3& up) {
        m_listenerForward = forward.Normalized();
        m_listenerUp = up.Normalized();
        
    }
    
    void AudioManager::CleanupFinishedSources() {
        m_audioSources.erase(
            std::remove_if(m_audioSources.begin(), m_audioSources.end(),
                [](const std::unique_ptr<AudioSource>& source) {
                    return source->IsStopped() && !source->IsLooping();
                }),
            m_audioSources.end()
        );
    }
}
