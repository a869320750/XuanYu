#pragma once
#include "SecureBase.h"

class SecureClient : public SecureBase {
public:
    SecureClient(std::shared_ptr<ITransportAdapter> t, std::shared_ptr<ICryptoProvider> c)
        : SecureBase(t, c) {}
    bool startHandshake() override { return true; }
    bool send(const std::vector<uint8_t>& plaintext) override { return true; }
    bool recv(std::vector<uint8_t>& plaintext) override { return true; }
};
