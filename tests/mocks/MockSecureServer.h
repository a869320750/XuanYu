#pragma once

#include "communication/SecureServer.h"
#include <gmock/gmock.h>

namespace xuanyu {
namespace tests {
namespace mocks {

/**
 * @brief SecureServer的Mock实现，用于单元测试
 */
class MockSecureServer : public communication::SecureServer {
public:
    MockSecureServer() = default;
    virtual ~MockSecureServer() {
        // 避免在析构函数中调用虚函数
        running_ = false;
        clientCount_ = 0;
    }

    // 实现SecureBase的纯虚函数
    virtual bool isConnected() const override {
        return running_;
    }

    virtual communication::ConnectionState getConnectionState() const override {
        return running_ ? communication::ConnectionState::Connected : communication::ConnectionState::Disconnected;
    }

    virtual void disconnect() override {
        running_ = false;
        clientCount_ = 0;
    }

    virtual bool sendMessage(int messageType, const std::vector<uint8_t>& data) override {
        (void)messageType; (void)data; // 抑制未使用参数警告
        return running_ && clientCount_ > 0;
    }

    virtual std::future<bool> sendMessageAsync(int messageType, const std::vector<uint8_t>& data) override {
        std::promise<bool> promise;
        promise.set_value(sendMessage(messageType, data));
        return promise.get_future();
    }

    virtual communication::SessionInfo getSessionInfo() const override {
        communication::SessionInfo info;
        info.sessionId = "server-session";
        info.protocolVersion = communication::ProtocolVersion{1, 0};
        info.establishedTime = std::chrono::system_clock::now();
        info.isActive = running_;
        return info;
    }

    virtual std::vector<communication::ProtocolVersion> getSupportedVersions() const override {
        return {communication::ProtocolVersion{1, 0}};
    }

    virtual void setMessageCallback(communication::MessageCallback callback) override {
        messageCallback_ = callback;
    }

    virtual void setErrorCallback(communication::ErrorCallback callback) override {
        errorCallback_ = callback;
    }

    virtual void setConnectionStateCallback(communication::ConnectionStateCallback callback) override {
        stateCallback_ = callback;
    }

    virtual std::string getLastError() const override {
        return lastError_;
    }

    virtual int getLastErrorCode() const override {
        return lastErrorCode_;
    }

    // 实现SecureServer的接口
    virtual bool start(const std::string& bindAddress, int listenPort) override {
        if (bindAddress.empty() || listenPort <= 0) {
            return false;
        }
        running_ = true;
        bindAddress_ = bindAddress;
        listenPort_ = listenPort;
        return true;
    }

    virtual void stop() override {
        running_ = false;
        clientCount_ = 0;
    }

    virtual bool isRunning() const override {
        return running_;
    }

    virtual int getClientCount() const override {
        return clientCount_;
    }

    virtual std::vector<int> getClientList() const override {
        std::vector<int> clients;
        for (int i = 1; i <= clientCount_; ++i) {
            clients.push_back(i);
        }
        return clients;
    }

    virtual communication::ClientInfo getClientInfo(int clientId) const override {
        (void)clientId; // 抑制未使用参数警告
        communication::ClientInfo info;
        // Mock implementation
        return info;
    }

    virtual bool disconnectClient(int clientId) override {
        if (clientId > 0 && clientCount_ > 0) {
            clientCount_--;
            return true;
        }
        return false;
    }

    virtual bool sendToClient(int clientId, int messageType, const std::vector<uint8_t>& data) override {
        (void)messageType; (void)data; // 抑制未使用参数警告
        return running_ && clientId > 0 && clientId <= clientCount_;
    }

    virtual int broadcast(int messageType, const std::vector<uint8_t>& data) override {
        (void)messageType; (void)data; // 抑制未使用参数警告
        return running_ ? clientCount_ : 0;
    }

    virtual int multicast(const std::vector<int>& clientIds, int messageType, const std::vector<uint8_t>& data) override {
        (void)messageType; (void)data; // 抑制未使用参数警告
        int sent = 0;
        if (running_) {
            for (int id : clientIds) {
                if (id > 0 && id <= clientCount_) {
                    sent++;
                }
            }
        }
        return sent;
    }

    // 测试辅助方法
    void setRunning(bool running) { running_ = running; }
    void setClientCount(int count) { clientCount_ = count; }
    void setLastError(const std::string& error, int code = -1) {
        lastError_ = error;
        lastErrorCode_ = code;
    }

    void simulateMessage(int type, const std::vector<uint8_t>& data) {
        if (messageCallback_) {
            messageCallback_(type, data);
        }
    }

    void simulateError(int code, const std::string& message) {
        if (errorCallback_) {
            errorCallback_(code, message);
        }
    }

    void simulateClientConnect() {
        clientCount_++;
    }

    void simulateClientDisconnect() {
        if (clientCount_ > 0) {
            clientCount_--;
        }
    }

private:
    bool running_ = false;
    int clientCount_ = 0;
    std::string bindAddress_;
    int listenPort_ = 0;
    std::string lastError_;
    int lastErrorCode_ = 0;

    communication::MessageCallback messageCallback_;
    communication::ErrorCallback errorCallback_;
    communication::ConnectionStateCallback stateCallback_;
};

} // namespace mocks
} // namespace tests
} // namespace xuanyu