#include "communication/SecureBase.h"
#include <iostream>
#include <thread>

namespace xuanyu {
namespace communication {

// 内部实现结构体
struct SecureBase::Impl {
    std::shared_ptr<comm::ITransportAdapter> transport;
    std::shared_ptr<crypto::ICryptoProvider> crypto;
    
    ConnectionState state;
    std::string lastError;
    int lastErrorCode;
    
    SessionInfo sessionInfo;
    std::vector<ProtocolVersion> supportedVersions;
    
    MessageCallback messageCallback;
    ErrorCallback errorCallback;
    ConnectionStateCallback stateCallback;
    
    Impl() : state(ConnectionState::Disconnected), lastErrorCode(0) {
        supportedVersions.push_back(ProtocolVersion(1, 0, 0));
    }
};

SecureBase::SecureBase() : pImpl(std::make_unique<Impl>()) {
}

SecureBase::~SecureBase() = default;

bool SecureBase::isConnected() const {
    return pImpl->state == ConnectionState::Connected;
}

ConnectionState SecureBase::getConnectionState() const {
    return pImpl->state;
}

void SecureBase::disconnect() {
    if (pImpl->transport) {
        pImpl->transport->disconnect();
    }
    pImpl->state = ConnectionState::Disconnected;
}

bool SecureBase::sendMessage(int messageType, const std::vector<uint8_t>& data) {
    (void)messageType; (void)data; // 抑制未使用参数警告
    if (!isConnected()) {
        pImpl->lastError = "Not connected";
        pImpl->lastErrorCode = -1;
        return false;
    }
    
    // 占位符实现 - 直接发送数据
    if (pImpl->transport) {
        int result = pImpl->transport->send(data);
        return result > 0;
    }
    
    return false;
}

std::future<bool> SecureBase::sendMessageAsync(int messageType, const std::vector<uint8_t>& data) {
    std::promise<bool> promise;
    auto future = promise.get_future();
    
    // 简单的异步实现
    std::thread([this, messageType, data, promise = std::move(promise)]() mutable {
        bool result = sendMessage(messageType, data);
        promise.set_value(result);
    }).detach();
    
    return future;
}

SessionInfo SecureBase::getSessionInfo() const {
    return pImpl->sessionInfo;
}

std::vector<ProtocolVersion> SecureBase::getSupportedVersions() const {
    return pImpl->supportedVersions;
}

void SecureBase::setMessageCallback(MessageCallback callback) {
    pImpl->messageCallback = callback;
}

void SecureBase::setErrorCallback(ErrorCallback callback) {
    pImpl->errorCallback = callback;
}

void SecureBase::setConnectionStateCallback(ConnectionStateCallback callback) {
    pImpl->stateCallback = callback;
}

std::string SecureBase::getLastError() const {
    return pImpl->lastError;
}

int SecureBase::getLastErrorCode() const {
    return pImpl->lastErrorCode;
}

} // namespace communication
} // namespace xuanyu