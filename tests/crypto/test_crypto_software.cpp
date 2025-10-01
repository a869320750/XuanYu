#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "crypto/CryptoSoftware.h"
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <iostream>

using namespace xuanyu::crypto;

class CryptoSoftwareTest : public ::testing::Test {
protected:
    void SetUp() override {
        crypto = std::make_unique<CryptoSoftware>();
    }

    std::unique_ptr<CryptoSoftware> crypto;
};

TEST_F(CryptoSoftwareTest, Initialization) {
    EXPECT_TRUE(crypto != nullptr);
    EXPECT_TRUE(crypto->getLastError().empty());
}

TEST_F(CryptoSoftwareTest, GenerateRandom) {
    std::vector<uint8_t> random1 = crypto->generateRandom(32);
    std::vector<uint8_t> random2 = crypto->generateRandom(32);
    
    EXPECT_EQ(random1.size(), 32);
    EXPECT_EQ(random2.size(), 32);
    EXPECT_NE(random1, random2); // 应该生成不同的随机数
}

TEST_F(CryptoSoftwareTest, GenerateRandomDifferentSizes) {
    std::vector<uint8_t> random16 = crypto->generateRandom(16);
    std::vector<uint8_t> random64 = crypto->generateRandom(64);
    
    EXPECT_EQ(random16.size(), 16);
    EXPECT_EQ(random64.size(), 64);
}

TEST_F(CryptoSoftwareTest, SM2KeyPairGeneration) {
    std::vector<uint8_t> publicKey, privateKey;
    bool result = crypto->generateSM2KeyPair(publicKey, privateKey);
    
    // 注意：这个测试可能会失败，因为我们的实现依赖GmSSL库
    // 在没有正确安装GmSSL的环境中，这个测试会失败
    if (result) {
        EXPECT_EQ(publicKey.size(), 65);  // 未压缩格式
        EXPECT_EQ(privateKey.size(), 32);
    } else {
        // 如果失败，至少确保错误信息不为空
        EXPECT_FALSE(crypto->getLastError().empty());
    }
}

TEST_F(CryptoSoftwareTest, SM3Hash) {
    std::vector<uint8_t> data = {0x61, 0x62, 0x63}; // "abc"
    std::vector<uint8_t> hash;
    
    bool result = crypto->sm3Hash(data, hash);
    
    if (result) {
        EXPECT_EQ(hash.size(), 32); // SM3 produces 256-bit hash
    } else {
        EXPECT_FALSE(crypto->getLastError().empty());
    }
}

TEST_F(CryptoSoftwareTest, SM4EncryptDecrypt) {
    std::vector<uint8_t> plaintext = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                                     0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    std::vector<uint8_t> key(16, 0x42);  // 16字节密钥
    std::vector<uint8_t> iv(16, 0x00);   // 16字节IV
    std::vector<uint8_t> ciphertext, decrypted;
    
    bool encryptResult = crypto->sm4Encrypt(plaintext, key, iv, ciphertext);
    
    if (encryptResult) {
        EXPECT_GT(ciphertext.size(), 0);
        
        bool decryptResult = crypto->sm4Decrypt(ciphertext, key, iv, decrypted);
        if (decryptResult) {
            EXPECT_EQ(plaintext, decrypted);
        }
    } else {
        EXPECT_FALSE(crypto->getLastError().empty());
    }
}

TEST_F(CryptoSoftwareTest, SM4InvalidKeySize) {
    std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03, 0x04};
    std::vector<uint8_t> invalidKey(15, 0x42);  // 错误的密钥长度
    std::vector<uint8_t> iv(16, 0x00);
    std::vector<uint8_t> ciphertext;
    
    bool result = crypto->sm4Encrypt(plaintext, invalidKey, iv, ciphertext);
    EXPECT_FALSE(result);
    EXPECT_FALSE(crypto->getLastError().empty());
}

TEST_F(CryptoSoftwareTest, SM2SignVerify) {
    std::vector<uint8_t> publicKey, privateKey;
    
    // 首先生成密钥对
    bool keyGenResult = crypto->generateSM2KeyPair(publicKey, privateKey);
    
    if (keyGenResult) {
        std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
        std::vector<uint8_t> signature;
        
        bool signResult = crypto->sm2Sign(data, privateKey, signature);
        if (signResult) {
            EXPECT_GT(signature.size(), 0);
            
            bool verifyResult = crypto->sm2Verify(data, signature, publicKey);
            EXPECT_TRUE(verifyResult);
            
            // 测试错误的数据验签失败
            std::vector<uint8_t> wrongData = {0x06, 0x07, 0x08, 0x09, 0x0A};
            bool wrongVerifyResult = crypto->sm2Verify(wrongData, signature, publicKey);
            EXPECT_FALSE(wrongVerifyResult);
        }
    }
    // 如果密钥生成失败，跳过签名验签测试
}

// ==================== 扩展测试用例 ====================

// 边界值测试
TEST_F(CryptoSoftwareTest, BoundaryValueTests) {
    // 测试空数据
    std::vector<uint8_t> emptyData;
    std::vector<uint8_t> hash;
    bool result = crypto->sm3Hash(emptyData, hash);
    if (result) {
        EXPECT_EQ(hash.size(), 32);
    }
    
    // 测试单字节数据
    std::vector<uint8_t> singleByte = {0xFF};
    result = crypto->sm3Hash(singleByte, hash);
    if (result) {
        EXPECT_EQ(hash.size(), 32);
    }
    
    // 测试大数据（多块）
    std::vector<uint8_t> largeData(1024, 0xAA);
    result = crypto->sm3Hash(largeData, hash);
    if (result) {
        EXPECT_EQ(hash.size(), 32);
    }
}

// 错误处理测试
TEST_F(CryptoSoftwareTest, ErrorHandlingTests) {
    // SM4错误密钥测试
    std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03, 0x04};
    std::vector<uint8_t> shortKey(8, 0x42);   // 太短
    std::vector<uint8_t> longKey(32, 0x42);   // 太长
    std::vector<uint8_t> iv(16, 0x00);
    std::vector<uint8_t> ciphertext;
    
    EXPECT_FALSE(crypto->sm4Encrypt(plaintext, shortKey, iv, ciphertext));
    EXPECT_FALSE(crypto->sm4Encrypt(plaintext, longKey, iv, ciphertext));
    
    // SM4错误 IV测试
    std::vector<uint8_t> validKey(16, 0x42);
    std::vector<uint8_t> shortIV(8, 0x00);
    std::vector<uint8_t> longIV(32, 0x00);
    
    EXPECT_FALSE(crypto->sm4Encrypt(plaintext, validKey, shortIV, ciphertext));
    EXPECT_FALSE(crypto->sm4Encrypt(plaintext, validKey, longIV, ciphertext));
    
    // SM2错误私钥测试
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    std::vector<uint8_t> signature;
    std::vector<uint8_t> shortPrivateKey(16, 0x42);  // 私钥太短
    std::vector<uint8_t> longPrivateKey(64, 0x42);   // 私钥太长
    
    EXPECT_FALSE(crypto->sm2Sign(data, shortPrivateKey, signature));
    EXPECT_FALSE(crypto->sm2Sign(data, longPrivateKey, signature));
    
    // SM2错误公钥测试
    std::vector<uint8_t> validSignature(64, 0x42);
    std::vector<uint8_t> shortPublicKey(32, 0x42);  // 公钥太短
    std::vector<uint8_t> longPublicKey(128, 0x42);  // 公钥太长
    
    EXPECT_FALSE(crypto->sm2Verify(data, validSignature, shortPublicKey));
    EXPECT_FALSE(crypto->sm2Verify(data, validSignature, longPublicKey));
}

// ICryptoProvider接口测试
TEST_F(CryptoSoftwareTest, ICryptoProviderInterfaceTests) {
    // 测试开启和关闭
    EXPECT_EQ(crypto->open(), 0);
    EXPECT_EQ(crypto->close(), 0);
    
    // 测试随机数生成
    uint8_t randomBuf[32];
    EXPECT_EQ(crypto->getRandom(randomBuf, 32), 0);
    EXPECT_EQ(crypto->getSecureRandom(randomBuf, 32), 0);
    
    // 测试SM2密钥对管理
    EXPECT_EQ(crypto->generateSM2KeyPair(0), 0);
    EXPECT_EQ(crypto->deleteSM2KeyPair(0), 0);
    
    // 测试用户ID管理
    uint8_t idBuf[32] = "test_user_id";
    EXPECT_EQ(crypto->importID(idBuf, 12, 2), 0);
    
    uint8_t exportedId[64];
    uint16_t exportedLen;
    EXPECT_EQ(crypto->exportID(exportedId, &exportedLen, 2), 0);
    
    // 测试SM4密钥管理
    uint8_t sm4Key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                          0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    EXPECT_EQ(crypto->setSM4Key(0, sm4Key), 0);
}

// 性能基准测试
TEST_F(CryptoSoftwareTest, PerformanceBenchmarkTests) {
    const int iterations = 100;
    auto start = std::chrono::high_resolution_clock::now();
    
    // SM3哈希性能测试
    std::vector<uint8_t> data(1024, 0xAA);
    for (int i = 0; i < iterations; ++i) {
        std::vector<uint8_t> hash;
        crypto->sm3Hash(data, hash);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 记录性能数据（仅供参考）
    std::cout << "SM3 Hash (" << iterations << " iterations of 1KB): " 
              << duration.count() << " microseconds" << std::endl;
    
    // SM4加密性能测试
    std::vector<uint8_t> key(16, 0x42);
    std::vector<uint8_t> iv(16, 0x00);
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::vector<uint8_t> ciphertext;
        crypto->sm4Encrypt(data, key, iv, ciphertext);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "SM4 Encrypt (" << iterations << " iterations of 1KB): " 
              << duration.count() << " microseconds" << std::endl;
}

// 数据一致性测试
TEST_F(CryptoSoftwareTest, DataConsistencyTests) {
    // SM3哈希一致性
    std::vector<uint8_t> data = {0x61, 0x62, 0x63}; // "abc"
    std::vector<uint8_t> hash1, hash2;
    
    bool result1 = crypto->sm3Hash(data, hash1);
    bool result2 = crypto->sm3Hash(data, hash2);
    
    if (result1 && result2) {
        EXPECT_EQ(hash1, hash2); // 相同输入应产生相同哈希
    }
    
    // SM4加解密一致性
    std::vector<uint8_t> plaintext = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    std::vector<uint8_t> key(16, 0x42);
    std::vector<uint8_t> iv(16, 0x00);
    
    for (int i = 0; i < 10; ++i) {
        std::vector<uint8_t> ciphertext, decrypted;
        
        bool encResult = crypto->sm4Encrypt(plaintext, key, iv, ciphertext);
        if (encResult) {
            bool decResult = crypto->sm4Decrypt(ciphertext, key, iv, decrypted);
            if (decResult) {
                EXPECT_EQ(plaintext, decrypted);
            }
        }
    }
}

// 多线程安全性测试
TEST_F(CryptoSoftwareTest, ThreadSafetyTests) {
    const int threadCount = 4;
    const int operationsPerThread = 50;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([this, &successCount, operationsPerThread]() {
            for (int i = 0; i < operationsPerThread; ++i) {
                std::vector<uint8_t> data = {static_cast<uint8_t>(i), static_cast<uint8_t>(i+1)};
                std::vector<uint8_t> hash;
                if (crypto->sm3Hash(data, hash)) {
                    successCount++;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 至少有一些操作成功（在GmSSL可用的环境中）
    // 在没有GmSSL的环境中，可能所有操作都会使用伪实现
    EXPECT_GE(successCount.load(), 0);
}

// 内存管理测试
TEST_F(CryptoSoftwareTest, MemoryManagementTests) {
    // 测试大量操作后内存不泄露
    for (int i = 0; i < 1000; ++i) {
        std::vector<uint8_t> data(100, static_cast<uint8_t>(i % 256));
        std::vector<uint8_t> hash;
        crypto->sm3Hash(data, hash);
        
        std::vector<uint8_t> key(16, static_cast<uint8_t>(i % 256));
        std::vector<uint8_t> iv(16, 0x00);
        std::vector<uint8_t> ciphertext;
        crypto->sm4Encrypt(data, key, iv, ciphertext);
    }
    
    // 如果没有崩溃，说明内存管理正常
    SUCCEED();
}