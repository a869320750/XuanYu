#include <gtest/gtest.h>
#include "crypto/CryptoSoftware.h"
#include <vector>
#include <memory>
#include <chrono>
#include <cstring>
#include <cstdio>

using namespace xuanyu::crypto;

/**
 * @brief 软件加密实现兼容性测试
 * 
 * 该测试套件验证CryptoSoftware软件实现的功能完整性和API兼容性，
 * 为将来与硬件实现的集成做准备。当硬件实现完成后，
 * 可以扩展此测试来验证软硬件实现的一致性。
 */
class SoftwareCompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        software = std::make_unique<CryptoSoftware>();
        ASSERT_TRUE(software != nullptr);
    }
    
    void TearDown() override {
        if (software) {
            software.reset();
        }
    }

    std::unique_ptr<CryptoSoftware> software;
};

// SM3哈希功能完整性测试（简化版）
TEST_F(SoftwareCompatibilityTest, SM3HashFunctionality) {
    std::vector<uint8_t> testData = {0x61, 0x62, 0x63}; // "abc"
    
    // 测试软件实现
    std::vector<uint8_t> hash;
    bool result = software->sm3Hash(testData, hash);
    
    if (result) {
        EXPECT_EQ(hash.size(), 32); // SM3 produces 256-bit (32-byte) hash
        std::cout << "SM3 hash completed successfully" << std::endl;
    } else {
        std::cout << "SM3 hash failed: " << software->getLastError() << std::endl;
        EXPECT_TRUE(result); // 这会失败并显示错误
    }
}

// SM2密钥生成功能测试
TEST_F(SoftwareCompatibilityTest, SM2KeyGenerationFunctionality) {
    // 测试向量风格API
    std::vector<uint8_t> publicKey, privateKey;
    bool result = software->generateSM2KeyPair(publicKey, privateKey);
    
    ASSERT_TRUE(result);
    EXPECT_EQ(publicKey.size(), 65);   // 未压缩公钥格式
    EXPECT_EQ(privateKey.size(), 32);  // 私钥长度
    
    // 测试ICryptoProvider接口
    int apiResult = software->generateSM2KeyPair(0); // 使用槽位0
    EXPECT_EQ(apiResult, 0);
    
    // 验证可以导出公钥
    uint8_t exportedPubKey[65];
    int exportResult = software->exportSM2PubKey(exportedPubKey, 0);
    EXPECT_EQ(exportResult, 0);
    
    std::cout << "SM2 key generation successful" << std::endl;
    std::cout << "Public key length: " << publicKey.size() << " bytes" << std::endl;
    std::cout << "Private key length: " << privateKey.size() << " bytes" << std::endl;
}

// SM4加密功能测试
TEST_F(SoftwareCompatibilityTest, SM4EncryptionFunctionality) {
    std::vector<uint8_t> plaintext = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                                     0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    std::vector<uint8_t> key = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                               0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    std::vector<uint8_t> iv(16, 0x00);
    
    // 测试向量风格API
    std::vector<uint8_t> ciphertext;
    bool encResult = software->sm4Encrypt(plaintext, key, iv, ciphertext);
    ASSERT_TRUE(encResult);
    EXPECT_GT(ciphertext.size(), 0);
    
    // 测试解密
    std::vector<uint8_t> decrypted;
    bool decResult = software->sm4Decrypt(ciphertext, key, iv, decrypted);
    ASSERT_TRUE(decResult);
    EXPECT_EQ(plaintext, decrypted);
    
    // 测试ICryptoProvider接口
    int keyResult = software->setSM4Key(0, key.data());
    EXPECT_EQ(keyResult, 0);
    
    uint8_t apiCiphertext[32];
    uint8_t icv[16] = {0};
    int cryptoResult = software->sm4Crypto(0, 1, 1, icv, plaintext.data(), 
                                          plaintext.size(), apiCiphertext);
    EXPECT_EQ(cryptoResult, 0);
    
    std::cout << "SM4 encryption/decryption successful" << std::endl;
    std::cout << "Plaintext size: " << plaintext.size() << " bytes" << std::endl;
    std::cout << "Ciphertext size: " << ciphertext.size() << " bytes" << std::endl;
}

// API完整性测试
TEST_F(SoftwareCompatibilityTest, APICompletenessTest) {
    // 测试所有主要接口的可用性
    int successCount = 0;
    const int totalTests = 10;
    
    // 随机数生成测试
    uint8_t randomBuffer[32];
    if (software->getRandom(randomBuffer, 32) == 0) {
        successCount++;
    }
    
    // SM2密钥对操作测试
    if (software->generateSM2KeyPair(0) == 0) {
        successCount++;
    }
    
    if (software->deleteSM2KeyPair(0) == 0) {
        successCount++;
    }
    
    // 用户ID管理测试
    const char* testId = "testuser";
    if (software->importID(reinterpret_cast<const uint8_t*>(testId), strlen(testId), 0) == 0) {
        successCount++;
    }
    
    uint8_t exportedId[32];
    uint16_t idLen;
    if (software->exportID(exportedId, &idLen, 0) == 0) {
        successCount++;
    }
    
    // SM3哈希操作测试
    if (software->sm3Init() == 0) {
        successCount++;
    }
    
    const char* testMsg = "test";
    if (software->sm3Update(reinterpret_cast<const uint8_t*>(testMsg), strlen(testMsg)) == 0) {
        successCount++;
    }
    
    uint8_t hashResult[32];
    if (software->sm3Final(hashResult) == 0) {
        successCount++;
    }
    
    // SM4密钥管理测试
    uint8_t sm4Key[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                          0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    if (software->setSM4Key(0, sm4Key) == 0) {
        successCount++;
    }
    
    uint8_t icv[16] = {0};
    if (software->sm4Init(0, 1, 1, icv) == 0) {
        successCount++;
    }
    
    // 验证API完整性
    EXPECT_GE(successCount, totalTests * 0.8); // 至少80%的API可用
    
    std::cout << "API completeness: " << successCount << "/" << totalTests 
              << " (" << (double)successCount/totalTests * 100 << "%)" << std::endl;
}

// 性能基准测试
TEST_F(SoftwareCompatibilityTest, PerformanceBenchmarkTest) {
    const int iterations = 100;
    std::vector<uint8_t> testData(1024, 0xAA);
    
    // SM3哈希性能测试
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::vector<uint8_t> hash;
        software->sm3Hash(testData, hash);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto sm3Duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // SM4加密性能测试
    std::vector<uint8_t> key(16, 0x42);
    std::vector<uint8_t> iv(16, 0x00);
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::vector<uint8_t> ciphertext;
        software->sm4Encrypt(testData, key, iv, ciphertext);
    }
    end = std::chrono::high_resolution_clock::now();
    auto sm4Duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Performance Benchmarks:" << std::endl;
    std::cout << "SM3 Hash (" << iterations << " iterations of 1KB): " 
              << sm3Duration.count() << " μs" << std::endl;
    std::cout << "SM4 Encrypt (" << iterations << " iterations of 1KB): " 
              << sm4Duration.count() << " μs" << std::endl;
    
    // 基本性能要求（这些值对于软件实现来说应该是合理的）
    EXPECT_LT(sm3Duration.count(), 100000); // 100ms内完成100次哈希
    EXPECT_LT(sm4Duration.count(), 100000); // 100ms内完成100次加密
    
    // 计算吞吐量
    double sm3Throughput = (double)(iterations * 1024) / sm3Duration.count(); // MB/s
    double sm4Throughput = (double)(iterations * 1024) / sm4Duration.count(); // MB/s
    
    std::cout << "SM3 Throughput: " << sm3Throughput << " MB/s" << std::endl;
    std::cout << "SM4 Throughput: " << sm4Throughput << " MB/s" << std::endl;
}

// 错误处理完整性测试
TEST_F(SoftwareCompatibilityTest, ErrorHandlingTest) {
    // 测试各种无效输入的错误处理
    
    // 无效SM4密钥长度
    std::vector<uint8_t> invalidKey(8, 0x42); // 错误的密钥长度
    std::vector<uint8_t> plaintext = {0x01, 0x02, 0x03, 0x04};
    std::vector<uint8_t> iv(16, 0x00);
    std::vector<uint8_t> ciphertext;
    
    bool result = software->sm4Encrypt(plaintext, invalidKey, iv, ciphertext);
    EXPECT_FALSE(result); // 应该失败
    EXPECT_FALSE(software->getLastError().empty()); // 应该有错误信息
    
    // 无效SM2公钥长度测试
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    std::vector<uint8_t> signature(64, 0x42);
    std::vector<uint8_t> invalidPubKey(32, 0x42); // 错误的公钥长度
    
    result = software->sm2Verify(data, signature, invalidPubKey);
    EXPECT_FALSE(result); // 应该失败
    
    // 空数据测试
    std::vector<uint8_t> emptyData;
    std::vector<uint8_t> hash;
    result = software->sm3Hash(emptyData, hash);
    EXPECT_FALSE(result); // 应该失败
    
    // NULL指针测试
    int apiResult = software->getRandom(nullptr, 32);
    EXPECT_NE(apiResult, 0); // 应该失败
    
    apiResult = software->sm3Hash(nullptr, 10, nullptr);
    EXPECT_NE(apiResult, 0); // 应该失败
    
    std::cout << "Error handling tests completed successfully" << std::endl;
}

// 数据完整性验证测试
TEST_F(SoftwareCompatibilityTest, DataIntegrityTest) {
    // 测试加解密的数据完整性
    std::vector<uint8_t> originalData = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };
    std::vector<uint8_t> key(16, 0x42);
    std::vector<uint8_t> iv(16, 0x00);
    
    // SM4加解密完整性测试
    std::vector<uint8_t> ciphertext, decrypted;
    bool encResult = software->sm4Encrypt(originalData, key, iv, ciphertext);
    ASSERT_TRUE(encResult);
    
    bool decResult = software->sm4Decrypt(ciphertext, key, iv, decrypted);
    ASSERT_TRUE(decResult);
    EXPECT_EQ(originalData, decrypted);
    
    // SM2签名验证完整性测试
    std::vector<uint8_t> publicKey, privateKey;
    bool keyGenResult = software->generateSM2KeyPair(publicKey, privateKey);
    ASSERT_TRUE(keyGenResult);
    
    std::vector<uint8_t> signature;
    bool signResult = software->sm2Sign(originalData, privateKey, signature);
    ASSERT_TRUE(signResult);
    
    bool verifyResult = software->sm2Verify(originalData, signature, publicKey);
    EXPECT_TRUE(verifyResult);
    
    // 修改数据后验证应该失败
    std::vector<uint8_t> modifiedData = originalData;
    modifiedData[0] ^= 0x01; // 修改第一个字节
    bool tamperVerifyResult = software->sm2Verify(modifiedData, signature, publicKey);
    EXPECT_FALSE(tamperVerifyResult);
    
    // SM3哈希一致性测试
    std::vector<uint8_t> hash1, hash2;
    bool hashResult1 = software->sm3Hash(originalData, hash1);
    bool hashResult2 = software->sm3Hash(originalData, hash2);
    ASSERT_TRUE(hashResult1 && hashResult2);
    EXPECT_EQ(hash1, hash2); // 相同数据应产生相同哈希
    
    // 不同数据应产生不同哈希
    std::vector<uint8_t> hash3;
    bool hashResult3 = software->sm3Hash(modifiedData, hash3);
    ASSERT_TRUE(hashResult3);
    EXPECT_NE(hash1, hash3);
    
    std::cout << "Data integrity verification completed successfully" << std::endl;
    std::cout << "Original data size: " << originalData.size() << " bytes" << std::endl;
    std::cout << "SM4 ciphertext size: " << ciphertext.size() << " bytes" << std::endl;
    std::cout << "SM2 signature size: " << signature.size() << " bytes" << std::endl;
    std::cout << "SM3 hash size: " << hash1.size() << " bytes" << std::endl;
}