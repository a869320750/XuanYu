#pragma once

#include <array>
#include <vector>
#include <cstdint>
namespace mvp {

class IHardware {
public:
    virtual ~IHardware() = default;
    virtual int Open() = 0;
    virtual void Close() = 0;
    virtual int GetChipFirmwareVersion(unsigned char* buf) = 0;
    virtual int SM3_Hash(const unsigned char* data, int len, unsigned char* out) = 0;
    virtual int Dev_Auth() = 0;
    virtual int Import_SM4Key(const std::array<unsigned char,16>& key) = 0;
    virtual int SM4_Crypto(int keyIndex, unsigned char type, unsigned char mode, unsigned char* icv, const unsigned char* in, int inLen, unsigned char* out) = 0;

    // --- SM2 related (high-level operations, buffers are raw bytes) ---
    virtual int SM2_GenerateKey(uint8_t slot) = 0; // 在指定槽位生成密钥对
    virtual int SM2_ExportPublicKey(uint8_t slot, std::vector<uint8_t>& outPub) = 0; // 导出公钥到 outPub
    virtual int SM2_ImportPublicKey(uint8_t slot, const std::vector<uint8_t>& pub) = 0; // 导入公钥到槽位
    virtual int SM2_Encrypt(uint8_t slot, const std::vector<uint8_t>& plaintext, std::vector<uint8_t>& outCipher) = 0; // 用槽位公钥加密
    virtual int SM2_Decrypt(uint8_t slot, const std::vector<uint8_t>& cipher, std::vector<uint8_t>& outPlain) = 0; // 用槽位私钥解密
    virtual int SM2_Sign(uint8_t slot, const std::vector<uint8_t>& data, std::vector<uint8_t>& outSig) = 0; // 用槽位私钥签名
    virtual int SM2_Verify(const std::vector<uint8_t>& pubkey, const std::vector<uint8_t>& data, const std::vector<uint8_t>& sig) = 0; // 用公钥验签
};

} // namespace mvp
