#pragma once

#include <string>
#include <vector>

namespace GameEngine {
    
    enum class AudioFormat {
        Mono8,
        Mono16,
        Stereo8,
        Stereo16
    };
    
    class AudioClip {
    public:
        AudioClip();
        ~AudioClip();
        
        bool LoadFromFile(const std::string& filepath);
        void Unload();
        
        // Audio data access
        const std::vector<unsigned char>& GetData() const { return m_data; }
        AudioFormat GetFormat() const { return m_format; }
        int GetSampleRate() const { return m_sampleRate; }
        int GetChannels() const { return m_channels; }
        int GetBitsPerSample() const { return m_bitsPerSample; }
        float GetDuration() const { return m_duration; }
        
        // OpenAL buffer ID
        unsigned int GetBufferID() const { return m_bufferID; }
        
        bool IsLoaded() const { return m_loaded; }
        const std::string& GetFilepath() const { return m_filepath; }
        
    private:
        bool LoadWAV(const std::string& filepath);
        bool LoadMP3(const std::string& filepath);
        void CalculateDuration();
        
        std::string m_filepath;
        std::vector<unsigned char> m_data;
        AudioFormat m_format = AudioFormat::Mono16;
        int m_sampleRate = 44100;
        int m_channels = 1;
        int m_bitsPerSample = 16;
        float m_duration = 0.0f;
        
        unsigned int m_bufferID = 0;
        bool m_loaded = false;
    };
}
