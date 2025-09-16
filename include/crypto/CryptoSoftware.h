#pragma once
#include <cstddef>
#include "ICryptoProvider.h"
#include <vector>
#include <cstdint>

class CryptoSoftware : public ICryptoProvider {
public:
    CryptoSoftware();
    ~CryptoSoftware();

    bool sign(const std::vector<uint8_t>& data, std::vector<uint8_t>& signature) override;
    bool verify(const std::vector<uint8_t>& data, const std::vector<uint8_t>& signature,
                const std::vector<uint8_t>& pubKey) override;
    bool generateKeyPair(std::vector<uint8_t>& pubKey, std::vector<uint8_t>& priKey) override;
    bool computeSharedKey(const std::vector<uint8_t>& peerPubKey,
                          const std::vector<uint8_t>& myPriKey,
                          std::vector<uint8_t>& sharedKey) override;

    std::vector<uint8_t> hash(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> kdf(const std::vector<uint8_t>& secret, size_t keyLen) override;

    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& key,
                                 const std::vector<uint8_t>& plaintext) override;
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& key,
                                 const std::vector<uint8_t>& ciphertext) override;
};
