# XuanYu 高安全通信框架 玄羽

「玄羽」(XuanYu)：玄 = 深邃难测，羽 = 无人机的羽翼，寓意“深不可测的飞行通信保护”。

[![Build Status](https://github.com/xxx/xuanyu/workflows/CI/badge.svg)](https://github.com/xxx/xuanyu/actions)
[![Coverage](https://codecov.io/gh/xxx/xuanyu/branch/main/graph/badge.svg)](https://codecov.io/gh/xxx/xuanyu)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

## 📖 项目简介

XuanYu是一个基于国密算法（SM2/SM3/SM4）的高安全通信框架，采用分层架构设计，支持多种传输协议和软硬件加密实现的无缝切换。

### 🎯 核心特性

- **🔒 国密算法支持**: 完整实现SM2/SM3/SM4算法套件
- **🔌 传输协议无关**: 支持TCP/UDP/MQTT/HTTP等多种传输方式
- **⚡ 软硬件兼容**: 支持软件实现和硬件安全芯片切换
- **🔄 安全握手**: 基于临时密钥交换的安全握手协议
- **🛡️ 攻击防护**: 内置重放攻击、中间人攻击等防护机制
- **📊 高性能**: 优化的加密性能和并发处理能力

### 🏗️ 架构设计

```
┌────────────────────────────────────────────────────────────┐
│                    业务应用层                               │
├────────────────────────────────────────────────────────────┤
│               安全协议层 (SecureClient/Server)              │
├────────────────────────────────────────────────────────────┤
│      传输适配层           │         加密抽象层               │
│   (ITransportAdapter)    │      (ICryptoProvider)          │
│  ┌─────────────────────┐ │ ┌─────────────────────────────┐ │
│  │ Socket │ MQTT │HTTP │ │ │ Software │    Hardware      │ │
│  │        │      │     │ │ │ (GmSSL)  │  (安全芯片)       │ │
│  └─────────────────────┘ │ └─────────────────────────────┘ │
└────────────────────────────────────────────────────────────┘
```

## 🚀 快速开始

### 环境要求

- **编译器**: GCC 7+ 或 Clang 6+
- **构建工具**: CMake 3.15+
- **依赖库**: 
  - GmSSL 3.0+
  - GoogleTest (测试)
  - nlohmann/json (JSON处理)

### 📦 编译安装

#### 使用Docker (推荐)

```bash
# 克隆项目
git clone https://github.com/xxx/xuanyu.git
cd xuanyu

# 使用Docker构建和测试
docker-compose up --build test

# 运行示例
docker-compose up --build demo
```

#### 本地编译

```bash
# 安装依赖 (Ubuntu)
sudo apt-get update
sudo apt-get install build-essential cmake libssl-dev

# 编译GmSSL (如果系统未安装)
git clone https://github.com/guanzhi/GmSSL.git
cd GmSSL
mkdir build && cd build
cmake .. && make -j$(nproc) && sudo make install

# 编译XuanYu
cd xuanyu
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 运行测试
make test

# 安装
sudo make install
```

### 📝 使用示例

#### 基础Echo服务端

```cpp
#include "xuanyu/SecureServer.h"
#include "xuanyu/TransportSocket.h"
#include "xuanyu/CryptoSoftware.h"

int main() {
    // 创建传输适配器
    auto transport = std::make_shared<TransportSocket>();
    
    // 创建加密提供者
    auto crypto = std::make_shared<CryptoSoftware>();
    
    // 创建安全服务端
    SecureServer server(transport, crypto);
    
    // 启动监听
    transport->bind("0.0.0.0", 8080);
    
    while (true) {
        if (server.acceptConnection()) {
            // 执行安全握手
            if (server.startHandshake()) {
                std::vector<uint8_t> data;
                if (server.recv(data)) {
                    // Echo回显
                    server.send(data);
                }
            }
        }
    }
    
    return 0;
}
```

#### 基础客户端

```cpp
#include "xuanyu/SecureClient.h"
#include "xuanyu/TransportSocket.h"
#include "xuanyu/CryptoSoftware.h"

int main() {
    auto transport = std::make_shared<TransportSocket>();
    auto crypto = std::make_shared<CryptoSoftware>();
    
    SecureClient client(transport, crypto);
    
    // 连接服务器
    if (transport->connect("127.0.0.1", 8080)) {
        // 执行安全握手
        if (client.startHandshake()) {
            // 发送数据
            std::string message = "Hello, XuanYu!";
            std::vector<uint8_t> data(message.begin(), message.end());
            
            if (client.send(data)) {
                std::vector<uint8_t> response;
                if (client.recv(response)) {
                    std::string reply(response.begin(), response.end());
                    std::cout << "收到回复: " << reply << std::endl;
                }
            }
        }
    }
    
    return 0;
}
```

## 📚 文档

- [设计文档](design.md) - 详细的架构设计和接口规范
- [敏捷开发计划](docs/agile-plan.md) - 完整的开发计划和里程碑
- [用户故事清单](docs/user-stories.md) - 详细的需求分解和验收标准
- [API参考](docs/api-reference.md) - 完整的API文档
- [部署指南](docs/deployment.md) - 生产环境部署指南
- [故障排查](docs/troubleshooting.md) - 常见问题和解决方案

## 🧪 测试

### 运行测试

```bash
# 编译和运行所有测试
cd build
make test

# 运行特定测试套件
./tests/unit_tests
./tests/integration_tests
./tests/performance_tests

# 生成覆盖率报告
make coverage
```

### 性能基准

```bash
# 运行性能基准测试
./tests/benchmark_tests

# 使用perf进行详细分析
perf record ./tests/benchmark_tests
perf report
```

## 📈 性能指标

| 指标 | 目标值 | 当前值 |
|------|--------|--------|
| 握手延迟 | < 100ms (LAN) | ~50ms |
| SM4加密速度 | > 100MB/s | ~150MB/s |
| SM2签名速度 | < 10ms | ~5ms |
| 最大并发连接 | > 1000 | ~1500 |
| 内存使用 | < 50MB (1000连接) | ~35MB |

## 🔒 安全特性

- ✅ **完整的国密算法支持** (SM2/SM3/SM4)
- ✅ **安全的密钥交换** (基于SM2 ECDH)
- ✅ **重放攻击防护** (Nonce + 时间戳)
- ✅ **中间人攻击防护** (证书验证)
- ✅ **会话密钥管理** (定期轮换)
- ✅ **内存安全** (安全清理)
- ✅ **日志脱敏** (敏感数据保护)

## 🤝 贡献指南

我们欢迎各种形式的贡献！请阅读 [CONTRIBUTING.md](CONTRIBUTING.md) 了解详细的贡献流程。

### 开发流程

1. Fork 本仓库
2. 创建功能分支 (`git checkout -b feature/amazing-feature`)
3. 提交变更 (`git commit -m 'Add amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建 Pull Request

### 代码规范

- 遵循 [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- 确保所有测试通过
- 代码覆盖率不低于85%
- 通过静态代码分析检查

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。

## 🙏 致谢

- [GmSSL](https://github.com/guanzhi/GmSSL) - 国密算法实现
- [GoogleTest](https://github.com/google/googletest) - 测试框架
- [nlohmann/json](https://github.com/nlohmann/json) - JSON库

---

**⭐ 如果这个项目对你有帮助，请给我们一个 Star！**
