#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <cstdint>

namespace GameEngine {
    class UDPSocket;
    class TCPSocket;
    
    enum class NetworkMode {
        None,
        Server,
        Client
    };
    
    struct NetworkMessage {
        std::vector<uint8_t> data;
        std::string senderAddress;
        int senderPort;
        
        NetworkMessage() = default;
        NetworkMessage(const std::vector<uint8_t>& messageData, 
                      const std::string& address = "", int port = 0)
            : data(messageData), senderAddress(address), senderPort(port) {}
    };
    
    using NetworkMessageCallback = std::function<void(const NetworkMessage&)>;
    
    class NetworkManager {
    public:
        NetworkManager();
        ~NetworkManager();
        
        // Initialization
        bool InitializeAsServer(int port);
        bool InitializeAsClient(const std::string& serverAddress, int serverPort);
        void Shutdown();
        
        // UDP Communication
        bool SendUDP(const std::vector<uint8_t>& data);
        bool SendUDP(const std::vector<uint8_t>& data, const std::string& address, int port);
        std::vector<NetworkMessage> ReceiveUDP();
        
        // TCP Communication
        bool SendTCP(const std::vector<uint8_t>& data);
        std::vector<NetworkMessage> ReceiveTCP();
        
        // Connection management
        bool IsConnected() const;
        bool IsServer() const { return m_mode == NetworkMode::Server; }
        bool IsClient() const { return m_mode == NetworkMode::Client; }
        
        // Event callbacks
        void SetUDPMessageCallback(NetworkMessageCallback callback);
        void SetTCPMessageCallback(NetworkMessageCallback callback);
        void SetConnectionCallback(std::function<void(bool)> callback);
        
        // Update method for polling
        void Update();
        
        // Network statistics
        size_t GetBytesSent() const { return m_bytesSent; }
        size_t GetBytesReceived() const { return m_bytesReceived; }
        int GetConnectedClients() const { return m_connectedClients; }
        
        bool IsInitialized() const { return m_initialized; }
        
    private:
        void ProcessIncomingMessages();
        void HandleClientConnection();
        void HandleServerMessages();
        
        bool m_initialized = false;
        NetworkMode m_mode = NetworkMode::None;
        
        std::unique_ptr<UDPSocket> m_udpSocket;
        std::unique_ptr<TCPSocket> m_tcpSocket;
        
        std::string m_serverAddress;
        int m_serverPort = 0;
        int m_localPort = 0;
        
        // Statistics
        size_t m_bytesSent = 0;
        size_t m_bytesReceived = 0;
        int m_connectedClients = 0;
        
        // Callbacks
        NetworkMessageCallback m_udpCallback;
        NetworkMessageCallback m_tcpCallback;
        std::function<void(bool)> m_connectionCallback;
    };
}
