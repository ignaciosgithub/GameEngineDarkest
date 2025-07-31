#include "NetworkManager.h"
#include "UDPSocket.h"
#include "TCPSocket.h"
#include "../Core/Logging/Logger.h"
#include <algorithm>

namespace GameEngine {
    
    NetworkManager::NetworkManager() = default;
    
    NetworkManager::~NetworkManager() {
        Shutdown();
    }
    
    bool NetworkManager::InitializeAsServer(int port) {
        if (m_initialized) {
            Logger::Warning("NetworkManager already initialized");
            return true;
        }
        
        if (!Socket::InitializeNetworking()) {
            Logger::Error("Failed to initialize networking subsystem");
            return false;
        }
        
        m_udpSocket = std::make_unique<UDPSocket>();
        if (!m_udpSocket->Initialize()) {
            Logger::Error("Failed to initialize UDP socket");
            return false;
        }
        
        if (!m_udpSocket->Bind(port)) {
            Logger::Error("Failed to bind UDP socket to port " + std::to_string(port));
            return false;
        }
        
        m_tcpSocket = std::make_unique<TCPSocket>();
        if (!m_tcpSocket->Initialize()) {
            Logger::Error("Failed to initialize TCP socket");
            return false;
        }
        
        if (!m_tcpSocket->Bind(port)) {
            Logger::Error("Failed to bind TCP socket to port " + std::to_string(port));
            return false;
        }
        
        if (!m_tcpSocket->Listen()) {
            Logger::Error("Failed to listen on TCP socket");
            return false;
        }
        
        m_mode = NetworkMode::Server;
        m_localPort = port;
        m_initialized = true;
        
        Logger::Info("NetworkManager initialized as server on port " + std::to_string(port));
        return true;
    }
    
    bool NetworkManager::InitializeAsClient(const std::string& serverAddress, int serverPort) {
        if (m_initialized) {
            Logger::Warning("NetworkManager already initialized");
            return true;
        }
        
        if (!Socket::InitializeNetworking()) {
            Logger::Error("Failed to initialize networking subsystem");
            return false;
        }
        
        m_udpSocket = std::make_unique<UDPSocket>();
        if (!m_udpSocket->Initialize()) {
            Logger::Error("Failed to initialize UDP socket");
            return false;
        }
        
        m_tcpSocket = std::make_unique<TCPSocket>();
        if (!m_tcpSocket->Initialize()) {
            Logger::Error("Failed to initialize TCP socket");
            return false;
        }
        
        if (!m_tcpSocket->Connect(serverAddress, serverPort)) {
            Logger::Error("Failed to connect to server " + serverAddress + ":" + std::to_string(serverPort));
            return false;
        }
        
        m_mode = NetworkMode::Client;
        m_serverAddress = serverAddress;
        m_serverPort = serverPort;
        m_initialized = true;
        
        Logger::Info("NetworkManager initialized as client, connected to " + 
                    serverAddress + ":" + std::to_string(serverPort));
        return true;
    }
    
    void NetworkManager::Shutdown() {
        if (!m_initialized) return;
        
        if (m_udpSocket) {
            m_udpSocket->Close();
            m_udpSocket.reset();
        }
        
        if (m_tcpSocket) {
            m_tcpSocket->Close();
            m_tcpSocket.reset();
        }
        
        Socket::CleanupNetworking();
        
        m_mode = NetworkMode::None;
        m_initialized = false;
        m_connectedClients = 0;
        
        Logger::Info("NetworkManager shutdown successfully");
    }
    
    bool NetworkManager::SendUDP(const std::vector<uint8_t>& data) {
        if (!m_initialized || !m_udpSocket) {
            Logger::Error("NetworkManager not initialized or UDP socket not available");
            return false;
        }
        
        int bytesSent = 0;
        if (m_mode == NetworkMode::Client) {
            bytesSent = m_udpSocket->Send(data, m_serverAddress, m_serverPort);
        } else {
            Logger::Error("SendUDP without target address only supported in client mode");
            return false;
        }
        
        if (bytesSent > 0) {
            m_bytesSent += bytesSent;
            return true;
        }
        
        return false;
    }
    
    bool NetworkManager::SendUDP(const std::vector<uint8_t>& data, const std::string& address, int port) {
        if (!m_initialized || !m_udpSocket) {
            Logger::Error("NetworkManager not initialized or UDP socket not available");
            return false;
        }
        
        int bytesSent = m_udpSocket->Send(data, address, port);
        if (bytesSent > 0) {
            m_bytesSent += bytesSent;
            return true;
        }
        
        return false;
    }
    
    std::vector<NetworkMessage> NetworkManager::ReceiveUDP() {
        std::vector<NetworkMessage> messages;
        
        if (!m_initialized || !m_udpSocket) {
            return messages;
        }
        
        std::vector<uint8_t> buffer;
        std::string senderAddress;
        int senderPort;
        
        while (true) {
            int bytesReceived = m_udpSocket->ReceiveFrom(buffer, senderAddress, senderPort);
            if (bytesReceived <= 0) {
                break;
            }
            
            m_bytesReceived += bytesReceived;
            messages.emplace_back(buffer, senderAddress, senderPort);
            
            if (m_udpCallback) {
                m_udpCallback(messages.back());
            }
        }
        
        return messages;
    }
    
    bool NetworkManager::SendTCP(const std::vector<uint8_t>& data) {
        if (!m_initialized || !m_tcpSocket) {
            Logger::Error("NetworkManager not initialized or TCP socket not available");
            return false;
        }
        
        if (!m_tcpSocket->IsConnected()) {
            Logger::Error("TCP socket not connected");
            return false;
        }
        
        int bytesSent = m_tcpSocket->Send(data);
        if (bytesSent > 0) {
            m_bytesSent += bytesSent;
            return true;
        }
        
        return false;
    }
    
    std::vector<NetworkMessage> NetworkManager::ReceiveTCP() {
        std::vector<NetworkMessage> messages;
        
        if (!m_initialized || !m_tcpSocket) {
            return messages;
        }
        
        if (!m_tcpSocket->IsConnected()) {
            return messages;
        }
        
        std::vector<uint8_t> buffer;
        int bytesReceived = m_tcpSocket->Receive(buffer);
        
        if (bytesReceived > 0) {
            m_bytesReceived += bytesReceived;
            
            std::string peerAddress = m_tcpSocket->GetPeerAddress();
            int peerPort = m_tcpSocket->GetPeerPort();
            
            messages.emplace_back(buffer, peerAddress, peerPort);
            
            if (m_tcpCallback) {
                m_tcpCallback(messages.back());
            }
        }
        
        return messages;
    }
    
    bool NetworkManager::IsConnected() const {
        if (!m_initialized) return false;
        
        if (m_mode == NetworkMode::Client && m_tcpSocket) {
            return m_tcpSocket->IsConnected();
        } else if (m_mode == NetworkMode::Server) {
            return true; // Server is always "connected" if initialized
        }
        
        return false;
    }
    
    void NetworkManager::SetUDPMessageCallback(NetworkMessageCallback callback) {
        m_udpCallback = callback;
    }
    
    void NetworkManager::SetTCPMessageCallback(NetworkMessageCallback callback) {
        m_tcpCallback = callback;
    }
    
    void NetworkManager::SetConnectionCallback(std::function<void(bool)> callback) {
        m_connectionCallback = callback;
    }
    
    void NetworkManager::Update() {
        if (!m_initialized) return;
        
        ProcessIncomingMessages();
        
        if (m_mode == NetworkMode::Server) {
            HandleClientConnection();
        } else if (m_mode == NetworkMode::Client) {
            HandleServerMessages();
        }
    }
    
    void NetworkManager::ProcessIncomingMessages() {
        auto udpMessages = ReceiveUDP();
        
        auto tcpMessages = ReceiveTCP();
    }
    
    void NetworkManager::HandleClientConnection() {
        if (!m_tcpSocket || m_mode != NetworkMode::Server) return;
        
        auto newClient = m_tcpSocket->Accept();
        if (newClient) {
            m_connectedClients++;
            Logger::Info("New client connected. Total clients: " + std::to_string(m_connectedClients));
            
            if (m_connectionCallback) {
                m_connectionCallback(true);
            }
        }
    }
    
    void NetworkManager::HandleServerMessages() {
        if (m_mode != NetworkMode::Client) return;
        
        bool wasConnected = IsConnected();
        
        bool isConnected = IsConnected();
        if (wasConnected != isConnected && m_connectionCallback) {
            m_connectionCallback(isConnected);
        }
    }
}
