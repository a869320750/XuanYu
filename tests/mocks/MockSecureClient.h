#pragma once

#include "communication/SecureClient.h"
#include <gmock/gmock.h>

namespace xuanyu {
namespace tests {
namespace mocks {

/**
 * @brief SecureClient的Mock实现，用于单元测试
 */
class MockSecureClient : public communication::SecureClient {
public:
    MockSecureClient() = default;
    virtual ~MockSecureClient() = default;

    // 实现SecureBase的纯虚函数
    virtual bool isConnected() const override {
        return connected_;
    }

    virtual communication::ConnectionState getConnectionState() const override {
        return state_;
    }

    virtual void disconnect() override {
        connected_ = false;
        state_ = communication::ConnectionState::Disconnected;
    }

    virtual bool sendMessage(int messageType, const std::vector<uint8_t>& data) override {
        (void)messageType; (void)data; // 抑制未使用参数警告
        return connected_;
    }

    virtual std::future<bool> sendMessageAsync(int messageType, const std::vector<uint8_t>& data) override {
        (void)messageType; (void)data; // 抑制未使用参数警告
        std::promise<bool> promise;
        promise.set_value(connected_);
        return promise.get_future();
    }

    virtual communication::SessionInfo getSessionInfo() const override {
        communication::SessionInfo info;
        info.sessionId = "test-session";
        info.protocolVersion = communication::ProtocolVersion{1, 0};
        info.establishedTime = std::chrono::system_clock::now();
        info.isActive = true;
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

    // 实现SecureClient的纯虚函数
    virtual bool connect(const communication::ConnectionConfig& config) override {
        return connect(config.serverAddress, config.serverPort);
    }

    virtual bool connect(const std::string& serverAddress, int serverPort) override {
        (void)serverPort; // 抑制未使用参数警告
        if (serverAddress == "invalid.server.com") {
            return false;
        }
        connected_ = true;
        state_ = communication::ConnectionState::Connected;
        return true;
    }

    virtual std::future<bool> connectAsync(const communication::ConnectionConfig& config,
                                         communication::ConnectCallback callback) override {
        std::promise<bool> promise;
        bool result = connect(config.serverAddress, config.serverPort);
        if (callback) {
            callback(result, result ? "" : "Connection failed");
        }
        promise.set_value(result);
        return promise.get_future();
    }

    virtual void configureReconnect(const communication::ReconnectConfig& config) override {
        reconnectConfig_ = config;
    }

    virtual void setAutoReconnect(bool enable) override {
        autoReconnect_ = enable;
    }

    virtual bool reconnect() override {
        return connect("127.0.0.1", 8443);
    }

    virtual void configureCertificate(const communication::CertificateConfig& config) override {
        certConfig_ = config;
    }

    virtual void setCertificate(const std::string& certPath, const std::string& keyPath) override {
        certPath_ = certPath;
        keyPath_ = keyPath;
    }

    virtual void setServerVerification(bool verify) override {
        verifyServer_ = verify;
    }

    virtual void setNetworkStatusCallback(communication::NetworkStatusCallback callback) override {
        networkCallback_ = callback;
    }

    virtual std::chrono::milliseconds getLatency() const override {
        return std::chrono::milliseconds(10);
    }

    virtual int getConnectionQuality() const override {
        return 95; // Excellent quality
    }

    virtual void setPreferredProtocolVersion(const communication::ProtocolVersion& version) override {
        preferredVersion_ = version;
    }

    virtual communication::ProtocolVersion getNegotiatedVersion() const override {
        return communication::ProtocolVersion{1, 0};
    }

    // 测试辅助方法
    void setConnected(bool connected) { connected_ = connected; }
    void setState(communication::ConnectionState state) { state_ = state; }
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

private:
    bool connected_ = false;
    communication::ConnectionState state_ = communication::ConnectionState::Disconnected;
    std::string lastError_;
    int lastErrorCode_ = 0;
    bool autoReconnect_ = false;
    std::string certPath_;
    std::string keyPath_;
    bool verifyServer_ = true;
    communication::ReconnectConfig reconnectConfig_;
    communication::CertificateConfig certConfig_;
    communication::ProtocolVersion preferredVersion_{1, 0};

    communication::MessageCallback messageCallback_;
    communication::ErrorCallback errorCallback_;
    communication::ConnectionStateCallback stateCallback_;
    communication::NetworkStatusCallback networkCallback_;
};

} // namespace mocks
} // namespace tests
} // namespace xuanyu