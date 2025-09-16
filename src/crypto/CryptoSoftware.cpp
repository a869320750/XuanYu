
// Minimal CryptoSoftware placeholder
#include "crypto/CryptoSoftware.h"

CryptoSoftware::CryptoSoftware() = default;
CryptoSoftware::~CryptoSoftware() = default;

bool CryptoSoftware::sign(const std::vector<uint8_t>&, std::vector<uint8_t>&) { return false; }
bool CryptoSoftware::verify(const std::vector<uint8_t>&, const std::vector<uint8_t>&, const std::vector<uint8_t>&) { return false; }
bool CryptoSoftware::generateKeyPair(std::vector<uint8_t>&, std::vector<uint8_t>&) { return false; }
bool CryptoSoftware::computeSharedKey(const std::vector<uint8_t>&, const std::vector<uint8_t>&, std::vector<uint8_t>&) { return false; }

std::vector<uint8_t> CryptoSoftware::hash(const std::vector<uint8_t>&) { return {}; }
std::vector<uint8_t> CryptoSoftware::kdf(const std::vector<uint8_t>&, size_t) { return {}; }

std::vector<uint8_t> CryptoSoftware::encrypt(const std::vector<uint8_t>&, const std::vector<uint8_t>&) { return {}; }
std::vector<uint8_t> CryptoSoftware::decrypt(const std::vector<uint8_t>&, const std::vector<uint8_t>&) { return {}; }
#include "crypto/ICryptoProvider.h"
#include <vector>

class CryptoSoftwareImpl : public ICryptoProvider {
public:
    bool sign(const std::vector<uint8_t>& data, std::vector<uint8_t>& signature) override { return true; }
    bool verify(const std::vector<uint8_t>& data, const std::vector<uint8_t>& signature,
                const std::vector<uint8_t>& pubKey) override { return true; }
    bool generateKeyPair(std::vector<uint8_t>& pubKey, std::vector<uint8_t>& priKey) override { return true; }
    bool computeSharedKey(const std::vector<uint8_t>& peerPubKey,
                          const std::vector<uint8_t>& myPriKey,
                          std::vector<uint8_t>& sharedKey) override { return true; }
    std::vector<uint8_t> hash(const std::vector<uint8_t>& data) override { return {}; }
    std::vector<uint8_t> kdf(const std::vector<uint8_t>& secret, size_t keyLen) override { return {}; }
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& key,
                                 const std::vector<uint8_t>& plaintext) override { return {}; }
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& key,
                                 const std::vector<uint8_t>& ciphertext) override { return {}; }
};
