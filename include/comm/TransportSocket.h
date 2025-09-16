#pragma once
#include "ITransportAdapter.h"
#include <vector>
#include <cstdint>
#include <string>

class TransportSocket : public ITransportAdapter {
public:
    TransportSocket();
    virtual ~TransportSocket();
    bool connect(const std::string &address, int port) override;
    bool send(const std::vector<uint8_t> &data) override;
    bool recv(std::vector<uint8_t> &data) override;
    void close() override;
};
