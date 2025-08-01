#include "AudioClip.h"
#include "../Core/Logging/Logger.h"
#include <fstream>
#include <cstring>
#include <algorithm>

namespace GameEngine {
    
    AudioClip::AudioClip() = default;
    
    AudioClip::~AudioClip() {
        Unload();
    }
    
    bool AudioClip::LoadFromFile(const std::string& filepath) {
        if (m_loaded) {
            Logger::Warning("AudioClip already loaded: " + filepath);
            return true;
        }
        
        m_filepath = filepath;
        
        std::string extension = filepath.substr(filepath.find_last_of('.') + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        if (extension == "wav") {
            return LoadWAV(filepath);
        } else {
            Logger::Error("Unsupported audio format: " + extension);
            return false;
        }
    }
    
    void AudioClip::Unload() {
        if (!m_loaded) return;
        
        if (m_bufferID != 0) {
            m_bufferID = 0;
        }
        
        m_data.clear();
        m_loaded = false;
        m_duration = 0.0f;
        
        Logger::Debug("Unloaded audio clip: " + m_filepath);
    }
    
    bool AudioClip::LoadWAV(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            Logger::Error("Failed to open WAV file: " + filepath);
            return false;
        }
        
        struct WAVHeader {
            char riff[4];           // "RIFF"
            uint32_t fileSize;      // File size - 8
            char wave[4];           // "WAVE"
            char fmt[4];            // "fmt "
            uint32_t fmtSize;       // Format chunk size
            uint16_t audioFormat;   // Audio format (1 = PCM)
            uint16_t numChannels;   // Number of channels
            uint32_t sampleRate;    // Sample rate
            uint32_t byteRate;      // Byte rate
            uint16_t blockAlign;    // Block align
            uint16_t bitsPerSample; // Bits per sample
            char data[4];           // "data"
            uint32_t dataSize;      // Data size
        };
        
        WAVHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));
        
        if (!file.good()) {
            Logger::Error("Failed to read WAV header: " + filepath);
            return false;
        }
        
        if (std::strncmp(header.riff, "RIFF", 4) != 0 ||
            std::strncmp(header.wave, "WAVE", 4) != 0 ||
            std::strncmp(header.fmt, "fmt ", 4) != 0 ||
            std::strncmp(header.data, "data", 4) != 0) {
            Logger::Error("Invalid WAV file format: " + filepath);
            return false;
        }
        
        if (header.audioFormat != 1) {
            Logger::Error("Unsupported WAV audio format (not PCM): " + filepath);
            return false;
        }
        
        m_channels = header.numChannels;
        m_sampleRate = header.sampleRate;
        m_bitsPerSample = header.bitsPerSample;
        
        if (m_channels == 1) {
            m_format = (m_bitsPerSample == 8) ? AudioFormat::Mono8 : AudioFormat::Mono16;
        } else if (m_channels == 2) {
            m_format = (m_bitsPerSample == 8) ? AudioFormat::Stereo8 : AudioFormat::Stereo16;
        } else {
            Logger::Error("Unsupported number of channels: " + std::to_string(m_channels));
            return false;
        }
        
        m_data.resize(header.dataSize);
        file.read(reinterpret_cast<char*>(m_data.data()), header.dataSize);
        
        if (!file.good()) {
            Logger::Error("Failed to read WAV audio data: " + filepath);
            m_data.clear();
            return false;
        }
        
        file.close();
        
        CalculateDuration();
        
        
        m_loaded = true;
        Logger::Info("Loaded WAV file: " + filepath + 
                    " (" + std::to_string(m_channels) + " channels, " +
                    std::to_string(m_sampleRate) + " Hz, " +
                    std::to_string(m_bitsPerSample) + " bits, " +
                    std::to_string(m_duration) + "s)");
        
        return true;
    }
    
    void AudioClip::CalculateDuration() {
        if (m_data.empty() || m_sampleRate == 0 || m_channels == 0 || m_bitsPerSample == 0) {
            m_duration = 0.0f;
            return;
        }
        
        int bytesPerSample = m_bitsPerSample / 8;
        int totalSamples = static_cast<int>(m_data.size()) / (m_channels * bytesPerSample);
        m_duration = static_cast<float>(totalSamples) / static_cast<float>(m_sampleRate);
    }
}
