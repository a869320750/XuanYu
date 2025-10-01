#pragma once

#include "ITransportAdapter.h"
#include <string>
#include <memory>

namespace xuanyu {
namespace comm {

/**
 * @brief Socket传输适配器实现
 * 基于TCP Socket的传输层实现
 */
class TransportSocket : public ITransportAdapter {
public:
    TransportSocket();
    virtual ~TransportSocket();
    
    // 禁止拷贝和赋值
    TransportSocket(const TransportSocket&) = delete;
    TransportSocket& operator=(const TransportSocket&) = delete;
    
    // ITransportAdapter接口实现
    bool connect(const std::string& address, int port) override;
    void disconnect() override;
    int send(const void* data, size_t size) override;
    int receive(void* buffer, size_t size) override;
    bool isConnected() const override;
    std::string getLastError() const override;
    
    /**
     * @brief 设置超时时间
     * @param timeoutMs 超时时间（毫秒）
     */
    void setTimeout(int timeoutMs);
    
    /**
     * @brief 启用/禁用Nagle算法
     * @param enable 是否启用
     */
    void setNoDelay(bool enable);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace comm
} // namespace xuanyu