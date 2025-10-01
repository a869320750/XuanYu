# ITransportAdapter 接口文档

## 概述

`ITransportAdapter` 是XuanYu安全通信框架的传输层抽象接口，支持多种传输协议的无缝切换。该接口设计遵循以下原则：

- **协议无关性**: 支持TCP、UDP、Serial等多种传输协议
- **同步/异步支持**: 提供同步和异步两种操作模式
- **事件驱动**: 支持回调函数和事件通知机制
- **配置灵活**: 支持运行时配置参数调整
- **状态透明**: 提供完整的连接状态和统计信息

## 接口分类

### 1. 连接管理接口

| 方法 | 描述 | 模式 |
|------|------|------|
| `connect()` | 同步连接到指定地址 | 同步 |
| `connectAsync()` | 异步连接到指定地址 | 异步 |
| `disconnect()` | 断开连接 | 同步 |
| `isConnected()` | 检查连接状态 | 查询 |

### 2. 数据传输接口

| 方法 | 描述 | 模式 |
|------|------|------|
| `send(void*, size_t)` | 发送原始数据 | 同步 |
| `send(vector<uint8_t>)` | 发送数据向量 | 同步 |
| `sendAsync()` | 异步发送数据 | 异步 |
| `recv(void*, size_t)` | 接收到缓冲区 | 同步 |
| `recv(size_t)` | 接收到向量 | 同步 |
| `recvAsync()` | 异步接收数据 | 异步 |

### 3. 错误处理接口

| 方法 | 描述 | 模式 |
|------|------|------|
| `getLastErrorCode()` | 获取错误代码 | 查询 |
| `getLastError()` | 获取错误消息 | 查询 |
| `setErrorCallback()` | 设置错误回调 | 配置 |
| `setConnectionCallback()` | 设置连接状态回调 | 配置 |
| `setDataCallback()` | 设置数据接收回调 | 配置 |

### 4. 配置接口

| 方法 | 描述 |
|------|------|
| `setTimeout()` / `getTimeout()` | 超时时间配置 |
| `setSendBufferSize()` / `getSendBufferSize()` | 发送缓冲区配置 |
| `setRecvBufferSize()` / `getRecvBufferSize()` | 接收缓冲区配置 |
| `setKeepAlive()` / `setKeepAliveInterval()` | Keep-Alive配置 |

### 5. 状态查询接口

| 方法 | 描述 |
|------|------|
| `getLocalAddress()` / `getLocalPort()` | 本地地址信息 |
| `getRemoteAddress()` / `getRemotePort()` | 远程地址信息 |
| `getConnectedTime()` | 连接建立时间 |
| `getTotalBytesSent()` / `getTotalBytesReceived()` | 传输统计信息 |

## 使用示例

### 基本同步使用

```cpp
#include "comm/ITransportAdapter.h"
#include "comm/TransportSocket.h"  // TCP实现

using namespace xuanyu::comm;

// 创建TCP传输适配器
auto transport = std::make_unique<TransportSocket>();

// 配置参数
transport->setTimeout(std::chrono::milliseconds(5000));
transport->setSendBufferSize(8192);
transport->setRecvBufferSize(8192);

// 连接服务器
if (transport->connect("192.168.1.100", 8080)) {
    std::cout << "连接成功!" << std::endl;
    
    // 发送数据
    std::string message = "Hello, Server!";
    std::vector<uint8_t> data(message.begin(), message.end());
    
    int sent = transport->send(data);
    if (sent > 0) {
        std::cout << "发送了 " << sent << " 字节" << std::endl;
    }
    
    // 接收响应
    auto response = transport->recv(1024);
    if (!response.empty()) {
        std::string responseStr(response.begin(), response.end());
        std::cout << "接收到: " << responseStr << std::endl;
    }
    
    transport->disconnect();
} else {
    std::cerr << "连接失败: " << transport->getLastError() << std::endl;
}
```

### 异步使用模式

```cpp
#include "comm/ITransportAdapter.h"
#include <iostream>

using namespace xuanyu::comm;

auto transport = std::make_unique<TransportSocket>();

// 设置回调函数
transport->setConnectionCallback([](bool connected) {
    if (connected) {
        std::cout << "异步连接成功!" << std::endl;
    } else {
        std::cout << "连接断开!" << std::endl;
    }
});

transport->setErrorCallback([](int errorCode, const std::string& errorMessage) {
    std::cerr << "错误 [" << errorCode << "]: " << errorMessage << std::endl;
});

transport->setDataCallback([](const std::vector<uint8_t>& data) {
    std::string message(data.begin(), data.end());
    std::cout << "异步接收到数据: " << message << std::endl;
});

// 异步连接
auto connectFuture = transport->connectAsync("192.168.1.100", 8080);

// 可以在这里做其他事情...

// 等待连接结果
if (connectFuture.get()) {
    std::cout << "异步连接成功!" << std::endl;
    
    // 异步发送数据
    std::string message = "Hello, Async Server!";
    std::vector<uint8_t> data(message.begin(), message.end());
    
    auto sendFuture = transport->sendAsync(data);
    
    // 等待发送结果
    int sent = sendFuture.get();
    std::cout << "异步发送了 " << sent << " 字节" << std::endl;
}
```

### 服务端监听模式示例

```cpp
// 注意：ITransportAdapter主要用于客户端连接
// 服务端监听需要单独的接口，如ITransportListener
// 这里展示的是服务端接受连接后的处理

void handleClient(std::unique_ptr<ITransportAdapter> clientTransport) {
    // 配置客户端连接
    clientTransport->setTimeout(std::chrono::milliseconds(30000));
    clientTransport->setKeepAlive(true);
    clientTransport->setKeepAliveInterval(std::chrono::seconds(30));
    
    // 设置错误处理
    clientTransport->setErrorCallback([](int code, const std::string& msg) {
        std::cout << "客户端错误: " << msg << std::endl;
    });
    
    // 接收客户端数据
    while (clientTransport->isConnected()) {
        auto data = clientTransport->recv(4096);
        if (!data.empty()) {
            // 处理数据
            processClientData(data);
            
            // 发送响应
            std::vector<uint8_t> response = generateResponse(data);
            clientTransport->send(response);
        }
    }
    
    std::cout << "客户端断开连接" << std::endl;
    std::cout << "总计发送: " << clientTransport->getTotalBytesSent() << " 字节" << std::endl;
    std::cout << "总计接收: " << clientTransport->getTotalBytesReceived() << " 字节" << std::endl;
}
```

## 错误处理

### 错误代码定义

```cpp
namespace xuanyu {
namespace comm {

enum class TransportError {
    SUCCESS = 0,
    CONNECTION_FAILED = 1001,
    CONNECTION_TIMEOUT = 1002,
    CONNECTION_REFUSED = 1003,
    SEND_FAILED = 2001,
    SEND_TIMEOUT = 2002,
    RECV_FAILED = 3001,
    RECV_TIMEOUT = 3002,
    BUFFER_FULL = 4001,
    INVALID_PARAMETER = 5001,
    NOT_CONNECTED = 5002
};

} // namespace comm
} // namespace xuanyu
```

### 错误处理最佳实践

1. **始终检查返回值**: 同步操作要检查返回值和错误状态
2. **设置合适的超时**: 避免无限期阻塞
3. **使用回调处理异步错误**: 异步操作通过回调函数处理错误
4. **记录详细错误信息**: 便于问题诊断

## 实现指南

### 实现者需要注意的要点

1. **线程安全**: 所有公共方法都应该是线程安全的
2. **资源管理**: 正确管理连接资源，避免泄漏
3. **状态一致性**: 保证连接状态的一致性
4. **性能优化**: 使用适当的缓冲策略和异步I/O
5. **平台兼容性**: 支持多平台实现

### 典型实现类

- `TransportSocket`: TCP/IP套接字实现
- `TransportUDP`: UDP套接字实现  
- `TransportSerial`: 串口通信实现
- `TransportSHM`: 共享内存实现
- `TransportMock`: 测试模拟实现

## 设计模式

该接口采用了以下设计模式：

- **适配器模式**: 统一不同传输协议的接口
- **策略模式**: 允许运行时切换传输策略
- **观察者模式**: 通过回调函数通知状态变化
- **命令模式**: 异步操作通过future/promise实现

## 与其他组件的关系

```
┌─────────────────┐    uses    ┌──────────────────┐
│  SecureClient   │────────────▶│ ITransportAdapter│
│  SecureServer   │             │                  │
└─────────────────┘             └──────────────────┘
                                          ▲
                                          │implements
                              ┌───────────┴───────────┐
                              │                       │
                    ┌─────────────────┐    ┌─────────────────┐
                    │ TransportSocket │    │ TransportUDP    │
                    │    (TCP)        │    │    (UDP)        │
                    └─────────────────┘    └─────────────────┘
```

ITransportAdapter被SecureClient和SecureServer使用，提供底层传输能力，支持安全握手协议和加密通信。