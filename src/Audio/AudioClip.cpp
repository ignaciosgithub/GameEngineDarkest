#include "AudioClip.h"
#include "../Core/Logging/Logger.h"
#define MINIMP3_IMPLEMENTATION
#include "External/minimp3.h"
#include "External/minimp3_ex.h"
#ifdef OPENAL_AVAILABLE
#include <AL/al.h>
#endif
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
        } else if (extension == "mp3") {
            return LoadMP3(filepath);
        } else {
            Logger::Error("Unsupported audio format: " + extension);
            return false;
        }
    }
    
    void AudioClip::Unload() {
        if (!m_loaded) return;
        
#ifdef OPENAL_AVAILABLE
        if (m_bufferID != 0) {
            alDeleteBuffers(1, &m_bufferID);
            m_bufferID = 0;
        }
#else
        if (m_bufferID != 0) {
            m_bufferID = 0;
        }
#endif
        
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
        
#ifdef OPENAL_AVAILABLE
        alGenBuffers(1, &m_bufferID);
        ALenum format;
        if (m_channels == 1) {
            format = (m_bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
        } else {
            format = (m_bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
        }
        alBufferData(m_bufferID, format, m_data.data(), static_cast<ALsizei>(m_data.size()), m_sampleRate);
        
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            Logger::Error("OpenAL error loading WAV buffer: " + std::to_string(error));
            alDeleteBuffers(1, &m_bufferID);
            m_bufferID = 0;
            return false;
        }
#endif
        
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
    
    bool AudioClip::LoadMP3(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            Logger::Error("Failed to open MP3 file: " + filepath);
            return false;
        }
        
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<unsigned char> fileData(fileSize);
        file.read(reinterpret_cast<char*>(fileData.data()), fileSize);
        file.close();
        
        mp3dec_t mp3d;
        mp3dec_init(&mp3d);
        
        mp3dec_file_info_t info;
        int result = mp3dec_load_buf(&mp3d, fileData.data(), fileSize, &info, nullptr, nullptr);
        
        if (result != 0) {
            Logger::Error("Failed to decode MP3 file: " + filepath);
            return false;
        }
        
        if (info.samples == 0) {
            Logger::Error("No audio data found in MP3 file: " + filepath);
            free(info.buffer);
            return false;
        }
        
        m_channels = info.channels;
        m_sampleRate = info.hz;
        m_bitsPerSample = 16;
        
        if (m_channels == 1) {
            m_format = AudioFormat::Mono16;
        } else if (m_channels == 2) {
            m_format = AudioFormat::Stereo16;
        } else {
            Logger::Error("Unsupported number of channels in MP3: " + std::to_string(m_channels));
            free(info.buffer);
            return false;
        }
        
        size_t dataSize = info.samples * sizeof(mp3d_sample_t);
        m_data.resize(dataSize);
        std::memcpy(m_data.data(), info.buffer, dataSize);
        
        free(info.buffer);
        
        CalculateDuration();
        
#ifdef OPENAL_AVAILABLE
        alGenBuffers(1, &m_bufferID);
        ALenum format = (m_channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        alBufferData(m_bufferID, format, m_data.data(), static_cast<ALsizei>(m_data.size()), m_sampleRate);
        
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            Logger::Error("OpenAL error loading MP3 buffer: " + std::to_string(error));
            alDeleteBuffers(1, &m_bufferID);
            m_bufferID = 0;
            return false;
        }
#endif
        
        m_loaded = true;
        Logger::Info("Loaded MP3 file: " + filepath + 
                     " (" + std::to_string(m_channels) + " channels, " +
                     std::to_string(m_sampleRate) + " Hz, " +
                     std::to_string(m_bitsPerSample) + " bits, " +
                     std::to_string(m_duration) + "s)");
        
        return true;
    }
}
