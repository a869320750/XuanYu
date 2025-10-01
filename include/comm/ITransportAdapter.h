#pragma once

#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <chrono>
#include <future>

namespace xuanyu {
namespace comm {

/**
 * @brief 传输适配器接口
 * 定义了通信传输层的抽象接口，支持多种传输协议的无缝切换
 * 支持同步/异步操作模式，包含完整的连接管理、数据传输、错误处理和配置功能
 */
class ITransportAdapter {
public:
    /**
     * @brief 错误回调函数类型
     * @param errorCode 错误代码
     * @param errorMessage 错误消息
     */
    using ErrorCallback = std::function<void(int errorCode, const std::string& errorMessage)>;
    
    /**
     * @brief 连接状态回调函数类型
     * @param connected 是否已连接
     */
    using ConnectionCallback = std::function<void(bool connected)>;
    
    /**
     * @brief 数据接收回调函数类型
     * @param data 接收到的数据
     */
    using DataCallback = std::function<void(const std::vector<uint8_t>& data)>;

    virtual ~ITransportAdapter() = default;
    
    // ==================== 连接管理接口 ====================
    
    /**
     * @brief 连接到指定地址
     * @param address 连接地址
     * @param port 端口号
     * @return 连接是否成功
     */
    virtual bool connect(const std::string& address, int port) = 0;
    
    /**
     * @brief 异步连接到指定地址
     * @param address 连接地址
     * @param port 端口号
     * @return future对象，用于获取连接结果
     */
    virtual std::future<bool> connectAsync(const std::string& address, int port) = 0;
    
    /**
     * @brief 断开连接
     */
    virtual void disconnect() = 0;
    
    /**
     * @brief 检查连接状态
     * @return 是否已连接
     */
    virtual bool isConnected() const = 0;
    
    // ==================== 数据传输接口 ====================
    
    /**
     * @brief 发送数据（同步）
     * @param data 要发送的数据
     * @param size 数据大小
     * @return 实际发送的字节数，-1表示发送失败
     */
    virtual int send(const void* data, size_t size) = 0;
    
    /**
     * @brief 发送数据（向量版本）
     * @param data 要发送的数据向量
     * @return 实际发送的字节数，-1表示发送失败
     */
    virtual int send(const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief 异步发送数据
     * @param data 要发送的数据向量
     * @return future对象，用于获取发送结果（发送的字节数）
     */
    virtual std::future<int> sendAsync(const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief 接收数据（同步）
     * @param buffer 接收缓冲区
     * @param size 缓冲区大小
     * @return 实际接收的字节数，-1表示接收失败，0表示连接关闭
     */
    virtual int recv(void* buffer, size_t size) = 0;
    
    /**
     * @brief 接收数据（向量版本）
     * @param maxSize 最大接收字节数
     * @return 接收到的数据，空向量表示失败或连接关闭
     */
    virtual std::vector<uint8_t> recv(size_t maxSize) = 0;
    
    /**
     * @brief 异步接收数据
     * @param maxSize 最大接收字节数
     * @return future对象，用于获取接收到的数据
     */
    virtual std::future<std::vector<uint8_t>> recvAsync(size_t maxSize) = 0;
    
    // ==================== 错误处理接口 ====================
    
    /**
     * @brief 获取最后一次错误代码
     * @return 错误代码，0表示无错误
     */
    virtual int getLastErrorCode() const = 0;
    
    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息字符串
     */
    virtual std::string getLastError() const = 0;
    
    /**
     * @brief 设置错误回调函数
     * @param callback 错误回调函数
     */
    virtual void setErrorCallback(const ErrorCallback& callback) = 0;
    
    /**
     * @brief 设置连接状态回调函数
     * @param callback 连接状态回调函数
     */
    virtual void setConnectionCallback(const ConnectionCallback& callback) = 0;
    
    /**
     * @brief 设置数据接收回调函数（用于异步接收模式）
     * @param callback 数据接收回调函数
     */
    virtual void setDataCallback(const DataCallback& callback) = 0;
    
    // ==================== 配置接口 ====================
    
    /**
     * @brief 设置超时时间
     * @param timeout 超时时间（毫秒）
     */
    virtual void setTimeout(std::chrono::milliseconds timeout) = 0;
    
    /**
     * @brief 获取超时时间
     * @return 超时时间（毫秒）
     */
    virtual std::chrono::milliseconds getTimeout() const = 0;
    
    /**
     * @brief 设置发送缓冲区大小
     * @param size 缓冲区大小（字节）
     */
    virtual void setSendBufferSize(size_t size) = 0;
    
    /**
     * @brief 设置接收缓冲区大小
     * @param size 缓冲区大小（字节）
     */
    virtual void setRecvBufferSize(size_t size) = 0;
    
    /**
     * @brief 获取发送缓冲区大小
     * @return 缓冲区大小（字节）
     */
    virtual size_t getSendBufferSize() const = 0;
    
    /**
     * @brief 获取接收缓冲区大小
     * @return 缓冲区大小（字节）
     */
    virtual size_t getRecvBufferSize() const = 0;
    
    /**
     * @brief 启用/禁用 Keep-Alive
     * @param enable 是否启用
     */
    virtual void setKeepAlive(bool enable) = 0;
    
    /**
     * @brief 设置 Keep-Alive 间隔
     * @param interval 间隔时间（秒）
     */
    virtual void setKeepAliveInterval(std::chrono::seconds interval) = 0;
    
    // ==================== 状态查询接口 ====================
    
    /**
     * @brief 获取本地地址
     * @return 本地地址字符串
     */
    virtual std::string getLocalAddress() const = 0;
    
    /**
     * @brief 获取本地端口
     * @return 本地端口号
     */
    virtual int getLocalPort() const = 0;
    
    /**
     * @brief 获取远程地址
     * @return 远程地址字符串
     */
    virtual std::string getRemoteAddress() const = 0;
    
    /**
     * @brief 获取远程端口
     * @return 远程端口号
     */
    virtual int getRemotePort() const = 0;
    
    /**
     * @brief 获取连接建立时间
     * @return 连接建立时间点
     */
    virtual std::chrono::system_clock::time_point getConnectedTime() const = 0;
    
    /**
     * @brief 获取发送的总字节数
     * @return 发送的总字节数
     */
    virtual size_t getTotalBytesSent() const = 0;
    
    /**
     * @brief 获取接收的总字节数
     * @return 接收的总字节数
     */
    virtual size_t getTotalBytesReceived() const = 0;
};

} // namespace comm
} // namespace xuanyu