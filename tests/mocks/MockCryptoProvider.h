#pragma once

#include "crypto/ICryptoProvider.h"
#include <vector>
#include <string>

namespace xuanyu {
namespace tests {
namespace mocks {

/**
 * @brief Mock实现的ICryptoProvider，用于单元测试
 */
class MockCryptoProvider : public crypto::ICryptoProvider {
public:
    MockCryptoProvider() = default;
    virtual ~MockCryptoProvider() = default;

    // ==================== 设备管理 ====================
    int open() override { return 0; }
    int close() override { return 0; }

    // ==================== 随机数生成 ====================
    int getRandom(uint8_t* rndBuf, uint16_t rndByteLen) override {
        if (!rndBuf) return -1;
        for (uint16_t i = 0; i < rndByteLen; ++i) {
            rndBuf[i] = static_cast<uint8_t>(i & 0xFF);
        }
        return 0;
    }
    
    int getSecureRandom(uint8_t* rndBuf, uint16_t rndByteLen) override {
        return getRandom(rndBuf, rndByteLen);
    }

    // ==================== SM2密钥管理 ====================
    int generateSM2KeyPair(uint8_t keyPairIndex) override {
        if (keyPairIndex >= 4) return -1;
        return 0;
    }
    
    int deleteSM2KeyPair(uint8_t keyPairIndex) override {
        if (keyPairIndex >= 4) return -1;
        return 0;
    }
    
    int importSM2KeyPair(const uint8_t* priKeyBuf, const uint8_t* pubKeyBuf, uint8_t keyPairIndex) override {
        if (keyPairIndex >= 4 || !priKeyBuf || !pubKeyBuf) return -1;
        return 0;
    }
    
    int importSM2PubKey(const uint8_t* pubKeyBuf, uint8_t keyPairIndex) override {
        if (keyPairIndex >= 4 || !pubKeyBuf) return -1;
        return 0;
    }
    
    int importSM2PriKey(const uint8_t* priKeyBuf, uint8_t keyIndex) override {
        if (keyIndex >= 4 || !priKeyBuf) return -1;
        return 0;
    }
    
    int exportSM2PubKey(uint8_t* pubKeyBuf, uint8_t keyPairIndex) override {
        if (keyPairIndex >= 4 || !pubKeyBuf) return -1;
        // 填充模拟公钥数据
        for (int i = 0; i < 65; ++i) {
            pubKeyBuf[i] = static_cast<uint8_t>((keyPairIndex * 65 + i) & 0xFF);
        }
        return 0;
    }

    // ==================== SM2加解密 ====================
    int sm2Encrypt(uint8_t* cipher, const uint8_t* msg, uint16_t msgByteLen, uint8_t keyPairIndex) override {
        if (!cipher || !msg || keyPairIndex >= 4) return -1;
        // 简单模拟：将明文复制到密文并添加一些固定字节
        for (uint16_t i = 0; i < msgByteLen; ++i) {
            cipher[i] = msg[i] ^ 0xAA;
        }
        return 0;
    }
    
    int sm2Decrypt(uint8_t* msg, const uint8_t* cipher, uint16_t cipherByteLen, uint8_t keyPairIndex) override {
        if (!msg || !cipher || keyPairIndex >= 4) return -1;
        // 简单模拟：逆向加密过程
        for (uint16_t i = 0; i < cipherByteLen; ++i) {
            msg[i] = cipher[i] ^ 0xAA;
        }
        return 0;
    }

    // ==================== SM2签名验签 ====================
    int sm2Sign(uint8_t* signBuf, const uint8_t* msg, uint16_t msgByteLen, uint8_t keyPairIndex, uint8_t idIndex) override {
        if (!signBuf || !msg || keyPairIndex >= 4) return -1;
        // 生成64字节的模拟签名
        for (int i = 0; i < 64; ++i) {
            signBuf[i] = static_cast<uint8_t>((keyPairIndex + idIndex + msgByteLen + i) & 0xFF);
        }
        return 0;
    }
    
    int sm2Verify(const uint8_t* signBuf, const uint8_t* msg, uint16_t msgByteLen, uint8_t keyPairIndex, uint8_t idIndex) override {
        if (!signBuf || !msg || keyPairIndex >= 4) return -1;
        // 简单验证：检查签名是否匹配期望模式
        for (int i = 0; i < 64; ++i) {
            uint8_t expected = static_cast<uint8_t>((keyPairIndex + idIndex + msgByteLen + i) & 0xFF);
            if (signBuf[i] != expected) return -2;
        }
        return 0;
    }
    
    int sm2SignDigest(uint8_t* signBuf, const uint8_t* digest, uint8_t keyPairIndex) override {
        return sm2Sign(signBuf, digest, 32, keyPairIndex, 0);
    }
    
    int sm2VerifyDigest(const uint8_t* signBuf, const uint8_t* digest, uint8_t keyPairIndex) override {
        return sm2Verify(signBuf, digest, 32, keyPairIndex, 0);
    }

    // ==================== 用户ID管理 ====================
    int importID(const uint8_t* idBuf, uint16_t idByteLen, uint8_t idIndex) override {
        if (!idBuf || idIndex < 2 || idIndex > 3) return -1;
        return 0;
    }
    
    int exportID(uint8_t* idBuf, uint16_t* idByteLen, uint8_t idIndex) override {
        if (!idBuf || !idByteLen || idIndex < 2 || idIndex > 3) return -1;
        // 返回模拟ID
        const char* mockId = "MockUser";
        size_t len = strlen(mockId);
        if (*idByteLen < len) return -2;
        memcpy(idBuf, mockId, len);
        *idByteLen = static_cast<uint16_t>(len);
        return 0;
    }

    // ==================== SM3算法 ====================
    int sm3Init() override { return 0; }
    
    int sm3Update(const uint8_t* msgBuf, uint16_t msgByteLen) override {
        if (!msgBuf) return -1;
        return 0;
    }
    
    int sm3Final(uint8_t* hashBuf) override {
        if (!hashBuf) return -1;
        // 生成32字节的模拟哈希
        for (int i = 0; i < 32; ++i) {
            hashBuf[i] = static_cast<uint8_t>(i * 7 & 0xFF);
        }
        return 0;
    }
    
    int sm3Hash(const uint8_t* msgBuf, uint16_t msgByteLen, uint8_t* hashBuf) override {
        if (!msgBuf || !hashBuf) return -1;
        // 生成基于消息长度的模拟哈希
        for (int i = 0; i < 32; ++i) {
            hashBuf[i] = static_cast<uint8_t>((msgByteLen + i * 13) & 0xFF);
        }
        return 0;
    }

    // ==================== SM4密钥管理 ====================
    int setSM4Key(uint8_t keyIndex, const uint8_t* keyBuf) override {
        if (keyIndex >= 6 || !keyBuf) return -1;
        return 0;
    }

    // ==================== SM4算法 ====================
    int sm4Init(uint8_t keyIndex, uint8_t type, uint8_t mode, const uint8_t* icv) override {
        if (keyIndex >= 6) return -1;
        return 0;
    }
    
    int sm4Update(uint8_t keyIndex, const uint8_t* inputBuf, uint16_t msgByteLen, uint8_t* outputBuf) override {
        if (keyIndex >= 6 || !inputBuf || !outputBuf) return -1;
        // 简单XOR操作作为模拟加密
        for (uint16_t i = 0; i < msgByteLen; ++i) {
            outputBuf[i] = inputBuf[i] ^ static_cast<uint8_t>(keyIndex + 0x42);
        }
        return 0;
    }
    
    int sm4Final(uint8_t keyIndex) override {
        if (keyIndex >= 6) return -1;
        return 0;
    }
    
    int sm4Crypto(uint8_t keyIndex, uint8_t type, uint8_t mode, const uint8_t* icv, 
                 const uint8_t* inputBuf, uint16_t msgByteLen, uint8_t* outputBuf) override {
        return sm4Update(keyIndex, inputBuf, msgByteLen, outputBuf);
    }
};

} // namespace mocks
} // namespace tests
} // namespace xuanyu