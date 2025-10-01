#pragma once

#include "SecureBase.h"
#include <chrono>
#include <future>

namespace xuanyu {
namespace communication {

/**
 * @brief 连接配置结构
 */
struct ConnectionConfig {
    std::string serverAddress;
    int serverPort;
    std::chrono::milliseconds timeout;
    bool enableSSL;
    std::string certificatePath;
    std::string privateKeyPath;
    bool verifyServerCert;
    
    ConnectionConfig() 
        : serverPort(0), timeout(std::chrono::milliseconds(5000)), 
          enableSSL(true), verifyServerCert(true) {}
};

/**
 * @brief 重连配置结构
 */
struct ReconnectConfig {
    bool enableAutoReconnect;
    int maxRetryAttempts;
    std::chrono::milliseconds initialDelay;
    double backoffMultiplier;
    std::chrono::milliseconds maxDelay;
    
    ReconnectConfig()
        : enableAutoReconnect(true), maxRetryAttempts(5),
          initialDelay(std::chrono::milliseconds(1000)),
          backoffMultiplier(2.0),
          maxDelay(std::chrono::milliseconds(30000)) {}
};

/**
 * @brief 证书配置结构
 */
struct CertificateConfig {
    std::string clientCertPath;
    std::string clientKeyPath;
    std::string caCertPath;
    std::string certPassword;
    bool requireClientAuth;
    
    CertificateConfig() : requireClientAuth(false) {}
};

/**
 * @brief 网络状态枚举
 */
enum class NetworkStatus {
    Unknown,
    Connected,
    Disconnected,
    Poor,
    Good
};

/**
 * @brief 网络状态回调类型
 */
using NetworkStatusCallback = std::function<void(NetworkStatus status)>;

/**
 * @brief 连接结果回调类型
 */
using ConnectCallback = std::function<void(bool success, const std::string& errorMessage)>;

/**
 * @brief 安全客户端类
 * 提供客户端特有的功能，如主动连接、自动重连、服务端验证等
 */
class SecureClient : public SecureBase {
public:
    /**
     * @brief 构造函数
     */
    SecureClient();
    
    /**
     * @brief 析构函数
     */
    virtual ~SecureClient();
    
    // ==================== 客户端连接接口 ====================
    
    /**
     * @brief 连接到服务器（同步）
     * @param config 连接配置
     * @return 连接是否成功
     */
    virtual bool connect(const ConnectionConfig& config) = 0;
    
    /**
     * @brief 异步连接到服务器
     * @param config 连接配置
     * @param callback 连接结果回调
     * @return future对象
     */
    virtual std::future<bool> connectAsync(const ConnectionConfig& config,
                                         ConnectCallback callback = nullptr) = 0;
    
    /**
     * @brief 简化的连接接口
     * @param serverAddress 服务器地址
     * @param serverPort 服务器端口
     * @return 连接是否成功
     */
    virtual bool connect(const std::string& serverAddress, int serverPort) = 0;
    
    // ==================== 重连配置接口 ====================
    
    /**
     * @brief 配置自动重连
     * @param config 重连配置
     */
    virtual void configureReconnect(const ReconnectConfig& config) = 0;
    
    /**
     * @brief 启用/禁用自动重连
     * @param enable 是否启用
     */
    virtual void setAutoReconnect(bool enable) = 0;
    
    /**
     * @brief 手动触发重连
     * @return 重连是否成功
     */
    virtual bool reconnect() = 0;
    
    // ==================== 证书管理接口 ====================
    
    /**
     * @brief 配置证书
     * @param config 证书配置
     */
    virtual void configureCertificate(const CertificateConfig& config) = 0;
    
    /**
     * @brief 设置客户端证书
     * @param certPath 证书文件路径
     * @param keyPath 私钥文件路径
     */
    virtual void setCertificate(const std::string& certPath, const std::string& keyPath) = 0;
    
    /**
     * @brief 设置是否验证服务端证书
     * @param verify 是否验证
     */
    virtual void setServerVerification(bool verify) = 0;
    
    // ==================== 网络监控接口 ====================
    
    /**
     * @brief 设置网络状态回调
     * @param callback 网络状态回调函数
     */
    virtual void setNetworkStatusCallback(NetworkStatusCallback callback) = 0;
    
    /**
     * @brief 获取网络延迟
     * @return 延迟时间（毫秒）
     */
    virtual std::chrono::milliseconds getLatency() const = 0;
    
    /**
     * @brief 获取连接质量评分
     * @return 质量评分（0-100）
     */
    virtual int getConnectionQuality() const = 0;
    
    // ==================== 协议协商接口 ====================
    
    /**
     * @brief 设置首选协议版本
     * @param version 协议版本
     */
    virtual void setPreferredProtocolVersion(const ProtocolVersion& version) = 0;
    
    /**
     * @brief 获取协商后的协议版本
     * @return 协议版本
     */
    virtual ProtocolVersion getNegotiatedVersion() const = 0;
};

} // namespace communication
} // namespace xuanyu