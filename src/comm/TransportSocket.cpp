#include "comm/TransportSocket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <cstring>
#include <errno.h>

namespace xuanyu {
namespace comm {

class TransportSocket::Impl {
public:
    Impl() : sockfd(-1), connected(false), timeoutMs(5000) {}
    
    ~Impl() {
        if (sockfd != -1) {
            close(sockfd);
        }
    }
    
    int sockfd;
    bool connected;
    int timeoutMs;
    std::string lastError;
};

TransportSocket::TransportSocket() : pImpl(std::make_unique<Impl>()) {
}

TransportSocket::~TransportSocket() = default;

bool TransportSocket::connect(const std::string& address, int port) {
    if (pImpl->connected) {
        disconnect();
    }
    
    // 创建socket
    pImpl->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (pImpl->sockfd == -1) {
        pImpl->lastError = "Failed to create socket: " + std::string(strerror(errno));
        return false;
    }
    
    // 设置地址结构
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
        pImpl->lastError = "Invalid address: " + address;
        close(pImpl->sockfd);
        pImpl->sockfd = -1;
        return false;
    }
    
    // 连接
    if (::connect(pImpl->sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        pImpl->lastError = "Connection failed: " + std::string(strerror(errno));
        close(pImpl->sockfd);
        pImpl->sockfd = -1;
        return false;
    }
    
    pImpl->connected = true;
    pImpl->lastError.clear();
    return true;
}

void TransportSocket::disconnect() {
    if (pImpl->sockfd != -1) {
        close(pImpl->sockfd);
        pImpl->sockfd = -1;
    }
    pImpl->connected = false;
}

int TransportSocket::send(const void* data, size_t size) {
    if (!pImpl->connected || pImpl->sockfd == -1) {
        pImpl->lastError = "Socket not connected";
        return -1;
    }
    
    ssize_t bytesSent = ::send(pImpl->sockfd, data, size, MSG_NOSIGNAL);
    if (bytesSent < 0) {
        pImpl->lastError = "Send failed: " + std::string(strerror(errno));
        return -1;
    }
    
    return static_cast<int>(bytesSent);
}

int TransportSocket::receive(void* buffer, size_t size) {
    if (!pImpl->connected || pImpl->sockfd == -1) {
        pImpl->lastError = "Socket not connected";
        return -1;
    }
    
    ssize_t bytesReceived = recv(pImpl->sockfd, buffer, size, 0);
    if (bytesReceived < 0) {
        pImpl->lastError = "Receive failed: " + std::string(strerror(errno));
        return -1;
    } else if (bytesReceived == 0) {
        // 连接关闭
        pImpl->connected = false;
        return 0;
    }
    
    return static_cast<int>(bytesReceived);
}

bool TransportSocket::isConnected() const {
    return pImpl->connected && pImpl->sockfd != -1;
}

std::string TransportSocket::getLastError() const {
    return pImpl->lastError;
}

void TransportSocket::setTimeout(int timeoutMs) {
    pImpl->timeoutMs = timeoutMs;
    
    if (pImpl->sockfd != -1) {
        struct timeval timeout;
        timeout.tv_sec = timeoutMs / 1000;
        timeout.tv_usec = (timeoutMs % 1000) * 1000;
        
        setsockopt(pImpl->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(pImpl->sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    }
}

void TransportSocket::setNoDelay(bool enable) {
    if (pImpl->sockfd != -1) {
        int flag = enable ? 1 : 0;
        setsockopt(pImpl->sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    }
}

} // namespace comm
} // namespace xuanyu