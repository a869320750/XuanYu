#pragma once

#include "IHardware.h"
#include <array>
#include <string>

namespace mvp {

class HardwareAdapter : public IHardware {
public:
    int Open() override;
    void Close() override;
    int GetChipFirmwareVersion(unsigned char* buf) override;
    int SM3_Hash(const unsigned char* data, int len, unsigned char* out) override;
    int Dev_Auth() override;
    int Import_SM4Key(const std::array<unsigned char,16>& key) override;
    int SM4_Crypto(int keyIndex, unsigned char type, unsigned char mode, unsigned char* icv, const unsigned char* in, int inLen, unsigned char* out) override;

    int SM2_GenerateKey(uint8_t slot) override; // 在指定槽位生成密钥对
    int SM2_ExportPublicKey(uint8_t slot, std::vector<uint8_t>& outPub) override; // 导出公钥到 outPub
    int SM2_ImportPublicKey(uint8_t slot, const std::vector<uint8_t>& pub) override; // 导入公钥到槽位
    int SM2_Encrypt(uint8_t slot, const std::vector<uint8_t>& plaintext, std::vector<uint8_t>& outCipher) override; // 用槽位公钥加密
    int SM2_Decrypt(uint8_t slot, const std::vector<uint8_t>& cipher, std::vector<uint8_t>& outPlain) override; // 用槽位私钥解密
    int SM2_Sign(uint8_t slot, const std::vector<uint8_t>& data, std::vector<uint8_t>& outSig) override; // 用槽位私钥签名
    int SM2_Verify(const std::vector<uint8_t>& pubkey, const std::vector<uint8_t>& data, const std::vector<uint8_t>& sig) override; // 用公钥验签

};

} // namespace mvp
