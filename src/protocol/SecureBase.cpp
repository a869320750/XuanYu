// Minimal placeholder implementation for SecureBase
#include "protocol/SecureBase.h"

SecureBase::SecureBase(std::shared_ptr<ITransportAdapter> t, std::shared_ptr<ICryptoProvider> c)
    : transport(std::move(t)), crypto(std::move(c)), sessionKey(), handshakeDone(false) {}

