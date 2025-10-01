#pragma once

#include "ICryptoProvider.h"
#include <memory>
#include <array>
#include <map>
#include <vector>
#include <mutex>

namespace xuanyu {
namespace crypto {

/**
 * @brief 软件加密提供者实现
 * 模拟大唐硬件芯片的槽位管理模式，使用内存存储代替硬件存储
 * 保持与硬件接口的完全一致性
 */
class CryptoSoftware : public ICryptoProvider {
public:
    CryptoSoftware();
    virtual ~CryptoSoftware();
    
    // 禁止拷贝和赋值
    CryptoSoftware(const CryptoSoftware&) = delete;
    CryptoSoftware& operator=(const CryptoSoftware&) = delete;

    // ==================== 设备管理 ====================
    int open() override;
    int close() override;

    // ==================== 随机数生成 ====================
    int getRandom(uint8_t* rndBuf, uint16_t rndByteLen) override;
    int getSecureRandom(uint8_t* rndBuf, uint16_t rndByteLen) override;

    // ==================== SM2密钥管理 ====================
    int generateSM2KeyPair(uint8_t keyPairIndex) override;
    int deleteSM2KeyPair(uint8_t keyPairIndex) override;
    int importSM2KeyPair(const uint8_t* priKeyBuf, const uint8_t* pubKeyBuf, uint8_t keyPairIndex) override;
    int importSM2PubKey(const uint8_t* pubKeyBuf, uint8_t keyPairIndex) override;
    int importSM2PriKey(const uint8_t* priKeyBuf, uint8_t keyIndex) override;
    int exportSM2PubKey(uint8_t* pubKeyBuf, uint8_t keyPairIndex) override;

    // ==================== SM2加解密 ====================
    int sm2Encrypt(uint8_t* cipher, const uint8_t* msg, uint16_t msgByteLen, uint8_t keyPairIndex) override;
    int sm2Decrypt(uint8_t* msg, const uint8_t* cipher, uint16_t cipherByteLen, uint8_t keyPairIndex) override;

    // ==================== SM2签名验签 ====================
    int sm2Sign(uint8_t* signBuf, const uint8_t* msg, uint16_t msgByteLen, uint8_t keyPairIndex, uint8_t idIndex) override;
    int sm2Verify(const uint8_t* signBuf, const uint8_t* msg, uint16_t msgByteLen, uint8_t keyPairIndex, uint8_t idIndex) override;
    int sm2SignDigest(uint8_t* signBuf, const uint8_t* digest, uint8_t keyPairIndex) override;
    int sm2VerifyDigest(const uint8_t* signBuf, const uint8_t* digest, uint8_t keyPairIndex) override;

    // ==================== 用户ID管理 ====================
    int importID(const uint8_t* idBuf, uint16_t idByteLen, uint8_t idIndex) override;
    int exportID(uint8_t* idBuf, uint16_t* idByteLen, uint8_t idIndex) override;

    // ==================== SM3算法 ====================
    int sm3Init() override;
    int sm3Update(const uint8_t* msgBuf, uint16_t msgByteLen) override;
    int sm3Final(uint8_t* hashBuf) override;
    int sm3Hash(const uint8_t* msgBuf, uint16_t msgByteLen, uint8_t* hashBuf) override;

    // ==================== SM4密钥管理 ====================
    int setSM4Key(uint8_t keyIndex, const uint8_t* keyBuf) override;

    // ==================== SM4算法 ====================
    int sm4Init(uint8_t keyIndex, uint8_t type, uint8_t mode, const uint8_t* icv) override;
    int sm4Update(uint8_t keyIndex, const uint8_t* inputBuf, uint16_t msgByteLen, uint8_t* outputBuf) override;
    int sm4Final(uint8_t keyIndex) override;
    int sm4Crypto(uint8_t keyIndex, uint8_t type, uint8_t mode, const uint8_t* icv, 
                 const uint8_t* inputBuf, uint16_t msgByteLen, uint8_t* outputBuf) override;

public:
    // ==================== 内部数据结构 ====================
    
    /**
     * @brief SM2密钥对结构
     */
    struct SM2KeyPair {
        std::array<uint8_t, 32> privateKey;  // 私钥（32字节）
        std::array<uint8_t, 65> publicKey;   // 公钥（65字节，未压缩格式）
        bool hasPrivateKey = false;          // 是否有私钥
        bool hasPublicKey = false;           // 是否有公钥
        
        void clear() {
            privateKey.fill(0);
            publicKey.fill(0);
            hasPrivateKey = false;
            hasPublicKey = false;
        }
    };

    /**
     * @brief SM4密钥结构
     */
    struct SM4Key {
        std::array<uint8_t, 16> key;         // SM4密钥（16字节）
        uint8_t keyType = 1;                 // 密钥类型（0:SM1, 1:SM4）
        bool isValid = false;                // 是否有效
        
        void clear() {
            key.fill(0);
            keyType = 1;
            isValid = false;
        }
    };

    /**
     * @brief 用户ID结构
     */
    struct UserID {
        std::vector<uint8_t> data;           // ID数据
        bool isValid = false;                // 是否有效
        
        void clear() {
            data.clear();
            isValid = false;
        }
    };

    // ==================== 内部存储槽位 ====================
    
    bool isOpened_;                                    // 设备是否已打开
    std::array<uint8_t, 32> serialNumber_;            // 序列号（32字节）
    bool hasSerialNumber_;                             // 是否有序列号
    
    std::array<SM2KeyPair, 4> sm2KeyPairs_;           // SM2密钥对槽位（索引0~3）
    std::array<SM4Key, 6> sm4Keys_;                    // SM4密钥槽位（索引0~5）
    std::array<UserID, 4> userIDs_;                   // 用户ID槽位（索引0~3，实际使用2~3）
    std::map<uint8_t, std::vector<uint8_t>> userData_;// 用户数据槽位（动态索引）
    
    // SM3运算上下文
    void* sm3Context_;                                 // SM3上下文指针
    bool sm3Initialized_;                              // SM3是否已初始化
    
    // SM3-HMAC运算上下文
    void* sm3HmacContext_;                             // SM3-HMAC上下文指针
    bool sm3HmacInitialized_;                          // SM3-HMAC是否已初始化
    
    // 线程安全
    mutable std::mutex mutex_;                         // 保护内部状态的互斥锁
    
    // 错误状态
    int lastErrorCode_;                                // 最后错误代码
    
    // ==================== 辅助方法 ====================
    
    /**
     * @brief 设置错误状态
     * @param errorCode 错误代码
     */
    void setError(int errorCode);
    
    /**
     * @brief 检查SM2密钥对索引有效性
     * @param keyPairIndex 密钥对索引
     * @return 是否有效
     */
    bool isValidSM2KeyPairIndex(uint8_t keyPairIndex) const;
    
    /**
     * @brief 检查SM4密钥索引有效性
     * @param keyIndex 密钥索引
     * @return 是否有效
     */
    bool isValidSM4KeyIndex(uint8_t keyIndex) const;
    
    /**
     * @brief 检查用户ID索引有效性
     * @param idIndex ID索引
     * @return 是否有效
     */
    bool isValidUserIDIndex(uint8_t idIndex) const;
    
    /**
     * @brief 清空所有槽位数据
     */
    void clearAllSlots();
    
    // Helper：保留现有 vector 风格实现，测试和工具代码可直接调用
    std::vector<uint8_t> generateRandom(size_t size);

    bool generateSM2KeyPair(std::vector<uint8_t>& publicKey, 
                            std::vector<uint8_t>& privateKey);

    bool sm2Sign(const std::vector<uint8_t>& data,
                const std::vector<uint8_t>& privateKey,
                std::vector<uint8_t>& signature);

    bool sm2Verify(const std::vector<uint8_t>& data,
                  const std::vector<uint8_t>& signature,
                  const std::vector<uint8_t>& publicKey);

    bool sm4Encrypt(const std::vector<uint8_t>& plaintext,
                   const std::vector<uint8_t>& key,
                   const std::vector<uint8_t>& iv,
                   std::vector<uint8_t>& ciphertext);

    bool sm4Decrypt(const std::vector<uint8_t>& ciphertext,
                   const std::vector<uint8_t>& key,
                   const std::vector<uint8_t>& iv,
                   std::vector<uint8_t>& plaintext);

    bool sm3Hash(const std::vector<uint8_t>& data,
                std::vector<uint8_t>& hash);

    std::string getLastError() const;
    
    /**
     * @brief 初始化加密库
     * @return 是否成功
     */
    bool initialize();
    
    /**
     * @brief 清理加密库资源
     */
    void cleanup();
};

} // namespace crypto
} // namespace xuanyu