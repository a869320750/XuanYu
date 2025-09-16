#pragma once
#include <cstddef>
#include <vector>
#include <cstdint>

class ICryptoProvider {
public:
    virtual bool sign(const std::vector<uint8_t>& data, std::vector<uint8_t>& signature) = 0;
    virtual bool verify(const std::vector<uint8_t>& data, const std::vector<uint8_t>& signature,
                        const std::vector<uint8_t>& pubKey) = 0;
    virtual bool generateKeyPair(std::vector<uint8_t>& pubKey, std::vector<uint8_t>& priKey) = 0;
    virtual bool computeSharedKey(const std::vector<uint8_t>& peerPubKey,
                                  const std::vector<uint8_t>& myPriKey,
                                  std::vector<uint8_t>& sharedKey) = 0;

    virtual std::vector<uint8_t> hash(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> kdf(const std::vector<uint8_t>& secret, size_t keyLen) = 0;

    virtual std::vector<uint8_t> encrypt(const std::vector<uint8_t>& key,
                                         const std::vector<uint8_t>& plaintext) = 0;
    virtual std::vector<uint8_t> decrypt(const std::vector<uint8_t>& key,
                                         const std::vector<uint8_t>& ciphertext) = 0;

    virtual ~ICryptoProvider() {}
};
