#pragma once
#include <vector>
#include <cstdint>

class HandshakeContext {
public:
    HandshakeContext() = default;
    ~HandshakeContext() = default;
    // placeholder members
    std::vector<uint8_t> lastMessage;
};
