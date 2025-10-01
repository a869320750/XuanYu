#pragma once

#include "comm/ITransportAdapter.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

namespace xuanyu {
namespace comm {

/**
 * @brief Mock传输适配器实现
 * 用于单元测试和开发阶段的模拟实现
 * 支持完整的ITransportAdapter接口，可以模拟各种网络场景
 */
class MockTransportAdapter : public ITransportAdapter {
public:
    MockTransportAdapter();
    virtual ~MockTransportAdapter();

    // ==================== 连接管理接口 ====================
    bool connect(const std::string& address, int port) override;
    std::future<bool> connectAsync(const std::string& address, int port) override;
    void disconnect() override;
    bool isConnected() const override;

    // ==================== 数据传输接口 ====================
    int send(const void* data, size_t size) override;
    int send(const std::vector<uint8_t>& data) override;
    std::future<int> sendAsync(const std::vector<uint8_t>& data) override;
    int recv(void* buffer, size_t size) override;
    std::vector<uint8_t> recv(size_t maxSize) override;
    std::future<std::vector<uint8_t>> recvAsync(size_t maxSize) override;

    // ==================== 错误处理接口 ====================
    int getLastErrorCode() const override;
    std::string getLastError() const override;
    void setErrorCallback(const ErrorCallback& callback) override;
    void setConnectionCallback(const ConnectionCallback& callback) override;
    void setDataCallback(const DataCallback& callback) override;

    // ==================== 配置接口 ====================
    void setTimeout(std::chrono::milliseconds timeout) override;
    std::chrono::milliseconds getTimeout() const override;
    void setSendBufferSize(size_t size) override;
    void setRecvBufferSize(size_t size) override;
    size_t getSendBufferSize() const override;
    size_t getRecvBufferSize() const override;
    void setKeepAlive(bool enable) override;
    void setKeepAliveInterval(std::chrono::seconds interval) override;

    // ==================== 状态查询接口 ====================
    std::string getLocalAddress() const override;
    int getLocalPort() const override;
    std::string getRemoteAddress() const override;
    int getRemotePort() const override;
    std::chrono::system_clock::time_point getConnectedTime() const override;
    size_t getTotalBytesSent() const override;
    size_t getTotalBytesReceived() const override;

    // ==================== 测试辅助方法 ====================
    
    /**
     * @brief 模拟接收数据（从外部注入数据到接收队列）
     * @param data 要注入的数据
     */
    void simulateReceiveData(const std::vector<uint8_t>& data);
    
    /**
     * @brief 获取发送的数据（用于验证发送内容）
     * @return 所有发送的数据
     */
    std::vector<std::vector<uint8_t>> getSentData() const;
    
    /**
     * @brief 清空发送数据记录
     */
    void clearSentData();
    
    /**
     * @brief 设置连接延迟（模拟网络延迟）
     * @param delay 连接延迟时间
     */
    void setConnectionDelay(std::chrono::milliseconds delay);
    
    /**
     * @brief 设置发送延迟（模拟网络延迟）
     * @param delay 发送延迟时间
     */
    void setSendDelay(std::chrono::milliseconds delay);
    
    /**
     * @brief 设置接收延迟（模拟网络延迟）
     * @param delay 接收延迟时间
     */
    void setReceiveDelay(std::chrono::milliseconds delay);
    
    /**
     * @brief 模拟连接失败
     * @param shouldFail 是否应该失败
     * @param errorCode 失败时的错误代码
     * @param errorMessage 失败时的错误消息
     */
    void setConnectionFailure(bool shouldFail, int errorCode = 1001, 
                             const std::string& errorMessage = "Mock connection failed");
    
    /**
     * @brief 模拟发送失败
     * @param shouldFail 是否应该失败
     * @param errorCode 失败时的错误代码
     * @param errorMessage 失败时的错误消息
     */
    void setSendFailure(bool shouldFail, int errorCode = 2001, 
                       const std::string& errorMessage = "Mock send failed");
    
    /**
     * @brief 模拟接收失败
     * @param shouldFail 是否应该失败
     * @param errorCode 失败时的错误代码
     * @param errorMessage 失败时的错误消息
     */
    void setReceiveFailure(bool shouldFail, int errorCode = 3001, 
                          const std::string& errorMessage = "Mock receive failed");

private:
    // 状态管理
    std::atomic<bool> connected_;
    std::string remoteAddress_;
    int remotePort_;
    std::chrono::system_clock::time_point connectedTime_;
    
    // 统计信息
    std::atomic<size_t> totalBytesSent_;
    std::atomic<size_t> totalBytesReceived_;
    
    // 配置参数
    std::chrono::milliseconds timeout_;
    size_t sendBufferSize_;
    size_t recvBufferSize_;
    bool keepAlive_;
    std::chrono::seconds keepAliveInterval_;
    
    // 错误状态
    std::atomic<int> lastErrorCode_;
    std::string lastErrorMessage_;
    mutable std::mutex errorMutex_;
    
    // 回调函数
    ErrorCallback errorCallback_;
    ConnectionCallback connectionCallback_;
    DataCallback dataCallback_;
    mutable std::mutex callbackMutex_;
    
    // 数据队列
    std::queue<std::vector<uint8_t>> receiveQueue_;
    std::vector<std::vector<uint8_t>> sentData_;
    mutable std::mutex dataQueueMutex_;
    std::condition_variable dataAvailable_;
    
    // 模拟参数
    std::chrono::milliseconds connectionDelay_;
    std::chrono::milliseconds sendDelay_;
    std::chrono::milliseconds receiveDelay_;
    
    bool connectionShouldFail_;
    int connectionFailureCode_;
    std::string connectionFailureMessage_;
    
    bool sendShouldFail_;
    int sendFailureCode_;
    std::string sendFailureMessage_;
    
    bool receiveShouldFail_;
    int receiveFailureCode_;
    std::string receiveFailureMessage_;
    
    mutable std::mutex configMutex_;
    
    // 辅助方法
    void setError(int errorCode, const std::string& errorMessage);
    void notifyError(int errorCode, const std::string& errorMessage);
    void notifyConnectionStatus(bool connected);
    void simulateDelay(std::chrono::milliseconds delay);
};

} // namespace comm
} // namespace xuanyu