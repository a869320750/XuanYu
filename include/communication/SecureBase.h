#pragma once

#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <cstdint>
#include "comm/ITransportAdapter.h"
#include "crypto/ICryptoProvider.h"

namespace xuanyu {
namespace communication {

/**
 * @brief 连接状态枚举
 */
enum class ConnectionState {
    Disconnected,    // 未连接
    Connecting,      // 连接中
    Handshaking,     // 握手中
    Connected,       // 已连接
    Error           // 错误状态
};

/**
 * @brief 协议版本结构
 */
struct ProtocolVersion {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    
    ProtocolVersion(uint8_t maj = 1, uint8_t min = 0, uint8_t pat = 0) 
        : major(maj), minor(min), patch(pat) {}
    
    bool operator==(const ProtocolVersion& other) const {
        return major == other.major && minor == other.minor && patch == other.patch;
    }
    
    bool operator<(const ProtocolVersion& other) const {
        if (major != other.major) return major < other.major;
        if (minor != other.minor) return minor < other.minor;
        return patch < other.patch;
    }
};

/**
 * @brief 会话信息结构
 */
struct SessionInfo {
    std::string sessionId;
    ProtocolVersion protocolVersion;
    std::chrono::system_clock::time_point establishedTime;
    std::vector<uint8_t> sessionKey;
    bool isActive;
    
    SessionInfo() : isActive(false) {}
};

/**
 * @brief 回调函数类型定义
 */
using MessageCallback = std::function<void(int messageType, const std::vector<uint8_t>& data)>;
using ErrorCallback = std::function<void(int errorCode, const std::string& errorMessage)>;
using ConnectionStateCallback = std::function<void(ConnectionState state)>;

/**
 * @brief 安全通信基础抽象类
 * 提供安全通信的核心功能，包括握手、消息传输、状态管理等
 */
class SecureBase {
public:
    virtual ~SecureBase();
    
    // ==================== 连接管理接口 ====================
    
    /**
     * @brief 检查连接状态
     * @return 是否已连接
     */
    virtual bool isConnected() const = 0;
    
    /**
     * @brief 获取连接状态
     * @return 当前连接状态
     */
    virtual ConnectionState getConnectionState() const = 0;
    
    /**
     * @brief 断开连接
     */
    virtual void disconnect() = 0;
    
    // ==================== 消息传输接口 ====================
    
    /**
     * @brief 发送消息
     * @param messageType 消息类型
     * @param data 消息数据
     * @return 发送是否成功
     */
    virtual bool sendMessage(int messageType, const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief 异步发送消息
     * @param messageType 消息类型
     * @param data 消息数据
     * @return future对象
     */
    virtual std::future<bool> sendMessageAsync(int messageType, const std::vector<uint8_t>& data) = 0;
    
    // ==================== 会话管理接口 ====================
    
    /**
     * @brief 获取会话信息
     * @return 当前会话信息
     */
    virtual SessionInfo getSessionInfo() const = 0;
    
    /**
     * @brief 获取支持的协议版本列表
     * @return 支持的版本列表
     */
    virtual std::vector<ProtocolVersion> getSupportedVersions() const = 0;
    
    // ==================== 回调设置接口 ====================
    
    /**
     * @brief 设置消息回调
     * @param callback 消息回调函数
     */
    virtual void setMessageCallback(MessageCallback callback) = 0;
    
    /**
     * @brief 设置错误回调
     * @param callback 错误回调函数
     */
    virtual void setErrorCallback(ErrorCallback callback) = 0;
    
    /**
     * @brief 设置连接状态回调
     * @param callback 连接状态回调函数
     */
    virtual void setConnectionStateCallback(ConnectionStateCallback callback) = 0;
    
    // ==================== 错误处理接口 ====================
    
    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息字符串
     */
    virtual std::string getLastError() const = 0;
    
    /**
     * @brief 获取错误码
     * @return 错误码
     */
    virtual int getLastErrorCode() const = 0;

protected:
    /**
     * @brief 内部实现结构体前向声明
     */
    struct Impl;
    std::unique_ptr<Impl> pImpl;
    
    /**
     * @brief 受保护的构造函数，只能被子类调用
     */
    SecureBase();
};

} // namespace communication
} // namespace xuanyu