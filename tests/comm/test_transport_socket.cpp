#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "comm/TransportSocket.h"

using namespace xuanyu::comm;

class TransportSocketTest : public ::testing::Test {
protected:
    void SetUp() override {
        transport = std::make_unique<TransportSocket>();
    }

    void TearDown() override {
        if (transport && transport->isConnected()) {
            transport->disconnect();
        }
    }

    std::unique_ptr<TransportSocket> transport;
};

TEST_F(TransportSocketTest, InitialState) {
    EXPECT_FALSE(transport->isConnected());
    EXPECT_TRUE(transport->getLastError().empty());
}

TEST_F(TransportSocketTest, ConnectToInvalidAddress) {
    bool result = transport->connect("invalid_address", 12345);
    EXPECT_FALSE(result);
    EXPECT_FALSE(transport->isConnected());
    EXPECT_FALSE(transport->getLastError().empty());
}

TEST_F(TransportSocketTest, SendWithoutConnection) {
    std::string data = "test data";
    int result = transport->send(data.c_str(), data.size());
    EXPECT_EQ(result, -1);
    EXPECT_FALSE(transport->getLastError().empty());
}

TEST_F(TransportSocketTest, ReceiveWithoutConnection) {
    char buffer[100];
    int result = transport->receive(buffer, sizeof(buffer));
    EXPECT_EQ(result, -1);
    EXPECT_FALSE(transport->getLastError().empty());
}

TEST_F(TransportSocketTest, TimeoutConfiguration) {
    // 测试超时设置不会崩溃
    EXPECT_NO_THROW(transport->setTimeout(5000));
}

TEST_F(TransportSocketTest, NoDelayConfiguration) {
    // 测试NoDelay设置不会崩溃
    EXPECT_NO_THROW(transport->setNoDelay(true));
    EXPECT_NO_THROW(transport->setNoDelay(false));
}

TEST_F(TransportSocketTest, DisconnectWithoutConnection) {
    // 测试在未连接状态下断开连接不会崩溃
    EXPECT_NO_THROW(transport->disconnect());
}

// 注意：实际的网络连接测试需要真实的服务器环境
// 这里只测试基本的错误处理和状态管理