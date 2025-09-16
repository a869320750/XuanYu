// Minimal TransportSocket placeholder
#include "comm/TransportSocket.h"
#include <iostream>

TransportSocket::TransportSocket() { }
TransportSocket::~TransportSocket() { }

bool TransportSocket::connect(const std::string &address, int port) {
    (void)address; (void)port;
    std::cout << "TransportSocket::connect placeholder" << std::endl;
    return false;
}

bool TransportSocket::send(const std::vector<uint8_t> &data) {
    (void)data;
    return false;
}

bool TransportSocket::recv(std::vector<uint8_t> &data) {
    (void)data;
    return false;
}

void TransportSocket::close() {
    // noop
}
