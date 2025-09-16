#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include "../comm/ITransportAdapter.h"
#include "../crypto/ICryptoProvider.h"

class SecureBase {
protected:
    std::shared_ptr<ITransportAdapter> transport;
    std::shared_ptr<ICryptoProvider> crypto;
    std::vector<uint8_t> sessionKey;
    bool handshakeDone = false;

public:
    SecureBase(std::shared_ptr<ITransportAdapter> t, std::shared_ptr<ICryptoProvider> c);
    virtual bool startHandshake() = 0;
    virtual bool send(const std::vector<uint8_t>& plaintext) = 0;
    virtual bool recv(std::vector<uint8_t>& plaintext) = 0;
    bool isHandshakeDone() const { return handshakeDone; }
    virtual ~SecureBase() {}
};
