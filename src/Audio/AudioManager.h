#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "../Core/Math/Vector3.h"

namespace GameEngine {
    class AudioClip;
    class AudioSource;
    
    class AudioManager {
    public:
        AudioManager();
        ~AudioManager();
        
        bool Initialize();
        void Update();
        void Shutdown();
        
        // Basic audio playback
        void PlaySound(const std::string& filepath, float volume = 1.0f);
        void PlaySound(AudioClip* clip, float volume = 1.0f);
        
        // 3D spatial audio
        void PlaySound3D(const std::string& filepath, const Vector3& position, float volume = 1.0f);
        void PlaySound3D(AudioClip* clip, const Vector3& position, float volume = 1.0f);
        
        // Audio clip management
        AudioClip* LoadAudioClip(const std::string& filepath);
        void UnloadAudioClip(const std::string& filepath);
        
        // Audio source management
        AudioSource* CreateAudioSource();
        void DestroyAudioSource(AudioSource* source);
        
        // Global audio settings
        void SetMasterVolume(float volume);
        float GetMasterVolume() const { return m_masterVolume; }
        
        void SetListenerPosition(const Vector3& position);
        void SetListenerOrientation(const Vector3& forward, const Vector3& up);
        
        bool IsInitialized() const { return m_initialized; }
        
    private:
        void CleanupFinishedSources();
        
        bool m_initialized = false;
        float m_masterVolume = 1.0f;
        
        Vector3 m_listenerPosition = Vector3::Zero;
        Vector3 m_listenerForward = Vector3(0, 0, -1);
        Vector3 m_listenerUp = Vector3(0, 1, 0);
        
        std::unordered_map<std::string, std::unique_ptr<AudioClip>> m_audioClips;
        std::vector<std::unique_ptr<AudioSource>> m_audioSources;
        
        // OpenAL context (will be implemented as void* for now)
        void* m_alDevice = nullptr;
        void* m_alContext = nullptr;
    };
}
