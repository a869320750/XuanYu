#pragma once

#include "SecureBase.h"
#include <map>
#include <set>

namespace xuanyu {
namespace communication {

/**
 * @brief 客户端连接信息
 */
struct ClientInfo {
    int clientId;
    std::string remoteAddress;
    int remotePort;
    std::chrono::system_clock::time_point connectedTime;
    SessionInfo session;
    bool isActive;
    
    ClientInfo() : clientId(-1), remotePort(0), isActive(false) {}
};

/**
 * @brief 服务器配置结构
 */
struct ServerConfig {
    std::string bindAddress;
    int listenPort;
    int maxConnections;
    std::chrono::milliseconds clientTimeout;
    bool enableSSL;
    std::string certificatePath;
    std::string privateKeyPath;
    bool requireClientAuth;
    
    ServerConfig()
        : bindAddress("0.0.0.0"), listenPort(0), maxConnections(1000),
          clientTimeout(std::chrono::milliseconds(30000)),
          enableSSL(true), requireClientAuth(false) {}
};

/**
 * @brief 客户端连接回调类型
 */
using ClientConnectedCallback = std::function<void(int clientId, const std::string& remoteAddress)>;
using ClientDisconnectedCallback = std::function<void(int clientId)>;
using ClientMessageCallback = std::function<void(int clientId, int messageType, const std::vector<uint8_t>& data)>;

/**
 * @brief 安全服务端类
 * 提供服务端特有的功能，如监听连接、多客户端管理、负载均衡等
 */
class SecureServer : public SecureBase {
public:
    /**
     * @brief 构造函数
     */
    SecureServer();
    
    /**
     * @brief 析构函数
     */
    virtual ~SecureServer();
    
    // ==================== 服务端监听接口 ====================
    
    /**
     * @brief 启动服务器监听
     * @param config 服务器配置
     * @return 启动是否成功
     */
    virtual bool start(const ServerConfig& config);
    
    /**
     * @brief 简化的启动接口
     * @param bindAddress 绑定地址
     * @param listenPort 监听端口
     * @return 启动是否成功
     */
    virtual bool start(const std::string& bindAddress, int listenPort);
    
    /**
     * @brief 停止服务器
     */
    virtual void stop();
    
    /**
     * @brief 检查服务器是否在运行
     * @return 是否运行中
     */
    virtual bool isRunning() const;
    
    // ==================== 客户端管理接口 ====================
    
    /**
     * @brief 获取已连接的客户端数量
     * @return 客户端数量
     */
    virtual int getClientCount() const;
    
    /**
     * @brief 获取所有客户端列表
     * @return 客户端ID列表
     */
    virtual std::vector<int> getClientList() const;
    
    /**
     * @brief 获取客户端信息
     * @param clientId 客户端ID
     * @return 客户端信息
     */
    virtual ClientInfo getClientInfo(int clientId) const;
    
    /**
     * @brief 断开指定客户端
     * @param clientId 客户端ID
     * @return 断开是否成功
     */
    virtual bool disconnectClient(int clientId);
    
    // ==================== 消息传输接口 ====================
    
    /**
     * @brief 向指定客户端发送消息
     * @param clientId 客户端ID
     * @param messageType 消息类型
     * @param data 消息数据
     * @return 发送是否成功
     */
    virtual bool sendToClient(int clientId, int messageType, const std::vector<uint8_t>& data);
    
    /**
     * @brief 广播消息给所有客户端
     * @param messageType 消息类型
     * @param data 消息数据
     * @return 成功发送的客户端数量
     */
    virtual int broadcast(int messageType, const std::vector<uint8_t>& data);
    
    /**
     * @brief 向指定客户端组发送消息
     * @param clientIds 客户端ID列表
     * @param messageType 消息类型
     * @param data 消息数据
     * @return 成功发送的客户端数量
     */
    virtual int multicast(const std::vector<int>& clientIds, int messageType, const std::vector<uint8_t>& data);
    
    // ==================== 客户端回调接口 ====================
    
    /**
     * @brief 设置客户端连接回调
     * @param callback 连接回调函数
     */
    virtual void setClientConnectedCallback(ClientConnectedCallback callback);
    
    /**
     * @brief 设置客户端断开回调
     * @param callback 断开回调函数
     */
    virtual void setClientDisconnectedCallback(ClientDisconnectedCallback callback);
    
    /**
     * @brief 设置客户端消息回调
     * @param callback 消息回调函数
     */
    virtual void setClientMessageCallback(ClientMessageCallback callback);
    
    // ==================== 证书管理接口 ====================
    
    /**
     * @brief 设置服务器证书
     * @param certPath 证书文件路径
     * @param keyPath 私钥文件路径
     */
    virtual void setCertificate(const std::string& certPath, const std::string& keyPath);
    
    /**
     * @brief 设置是否要求客户端认证
     * @param require 是否要求
     */
    virtual void setClientVerification(bool require);
    
    // ==================== 连接控制接口 ====================
    
    /**
     * @brief 设置最大连接数
     * @param maxConnections 最大连接数
     */
    virtual void setMaxConnections(int maxConnections);
    
    /**
     * @brief 设置客户端超时时间
     * @param timeout 超时时间
     */
    virtual void setClientTimeout(std::chrono::milliseconds timeout);
    
    /**
     * @brief 启用/禁用速率限制
     * @param enable 是否启用
     * @param maxMessagesPerSecond 每秒最大消息数
     */
    virtual void setRateLimit(bool enable, int maxMessagesPerSecond = 100);
};

} // namespace communication
} // namespace xuanyu