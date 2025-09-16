#pragma once
#include <vector>
#include <cstdint>
#include <string>

class ITransportAdapter {
public:
    virtual bool connect(const std::string& address, int port) = 0;
    virtual bool send(const std::vector<uint8_t>& data) = 0;
    virtual bool recv(std::vector<uint8_t>& data) = 0;
    virtual void close() = 0;
    virtual ~ITransportAdapter() {}
};
