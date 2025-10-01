#include "crypto/CryptoSoftware.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstring>

using namespace xuanyu::crypto;

// 辅助函数：打印十六进制数据
void printHex(const std::vector<uint8_t>& data, const std::string& prefix) {
    std::cout << prefix << ": ";
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
        if (i % 16 == 15) std::cout << std::endl << "          ";
    }
    std::cout << std::dec << std::endl;
}

void demonstrateSM3() {
    std::cout << "\n=== SM3 Hash Demonstration ===" << std::endl;
    
    CryptoSoftware crypto;
    
    // 测试数据
    std::string message = "This is a test message for SM3 hashing";
    std::vector<uint8_t> data(message.begin(), message.end());
    
    std::cout << "Message: " << message << std::endl;
    
    // 计算SM3哈希
    std::vector<uint8_t> hash;
    bool hashResult = crypto.sm3Hash(data, hash);
    
    if (hashResult) {
        printHex(hash, "SM3 Hash");
        
        // 验证哈希长度
        if (hash.size() == 32) {
            std::cout << "✓ SM3 hash length is correct (32 bytes)" << std::endl;
        } else {
            std::cout << "✗ SM3 hash length is incorrect (" << hash.size() << " bytes)" << std::endl;
        }
    } else {
        std::cout << "SM3 hashing failed: " << crypto.getLastError() << std::endl;
    }
}

void demonstrateSM4() {
    std::cout << "\n=== SM4 Encryption Demonstration ===" << std::endl;
    
    CryptoSoftware crypto;
    
    // 测试数据
    std::string message = "This is a test message for SM4 encryption";
    std::vector<uint8_t> plaintext(message.begin(), message.end());
    
    // 密钥和初始向量
    std::vector<uint8_t> key(16, 0x42);  // 简单的测试密钥
    std::vector<uint8_t> iv(16, 0x00);   // 初始向量
    
    std::cout << "Plaintext: " << message << std::endl;
    printHex(key, "Key");
    
    // 加密
    std::vector<uint8_t> ciphertext;
    bool encryptResult = crypto.sm4Encrypt(plaintext, key, iv, ciphertext);
    
    if (encryptResult) {
        printHex(ciphertext, "Ciphertext");
        
        // 解密
        std::vector<uint8_t> decryptedtext;
        bool decryptResult = crypto.sm4Decrypt(ciphertext, key, iv, decryptedtext);
        
        if (decryptResult) {
            std::string decryptedMessage(decryptedtext.begin(), decryptedtext.end());
            std::cout << "Decrypted: " << decryptedMessage << std::endl;
            
            if (message == decryptedMessage) {
                std::cout << "✓ SM4 encryption/decryption successful!" << std::endl;
            } else {
                std::cout << "✗ SM4 encryption/decryption failed - messages don't match!" << std::endl;
            }
        } else {
            std::cout << "Decryption failed: " << crypto.getLastError() << std::endl;
        }
    } else {
        std::cout << "Encryption failed: " << crypto.getLastError() << std::endl;
    }
}

void demonstrateSM2() {
    std::cout << "\n=== SM2 Digital Signature Demonstration ===" << std::endl;
    
    CryptoSoftware crypto;
    
    // 生成密钥对
    std::vector<uint8_t> publicKey, privateKey;
    bool keyGenResult = crypto.generateSM2KeyPair(publicKey, privateKey);
    
    if (!keyGenResult) {
        std::cout << "Key generation failed: " << crypto.getLastError() << std::endl;
        return;
    }
    
    printHex(publicKey, "Public Key");
    printHex(privateKey, "Private Key");
    
    // 要签名的消息
    std::string message = "This is a message to be signed with SM2";
    std::vector<uint8_t> data(message.begin(), message.end());
    
    std::cout << "Message: " << message << std::endl;
    
    // 数字签名
    std::vector<uint8_t> signature;
    bool signResult = crypto.sm2Sign(data, privateKey, signature);
    
    if (signResult) {
        printHex(signature, "Signature");
        
        // 验证签名
        bool verifyResult = crypto.sm2Verify(data, signature, publicKey);
        
        if (verifyResult) {
            std::cout << "✓ Signature verification successful!" << std::endl;
        } else {
            std::cout << "✗ Signature verification failed!" << std::endl;
        }
        
        // 测试错误的消息
        std::string wrongMessage = "This is a different message";
        std::vector<uint8_t> wrongData(wrongMessage.begin(), wrongMessage.end());
        bool wrongVerifyResult = crypto.sm2Verify(wrongData, signature, publicKey);
        
        if (!wrongVerifyResult) {
            std::cout << "✓ Wrong message correctly rejected!" << std::endl;
        } else {
            std::cout << "✗ Wrong message incorrectly accepted!" << std::endl;
        }
        
    } else {
        std::cout << "Signing failed: " << crypto.getLastError() << std::endl;
    }
}

void demonstrateICryptoProviderInterface() {
    std::cout << "\n=== ICryptoProvider Interface Demonstration ===" << std::endl;
    
    CryptoSoftware crypto;
    
    // 开启加密服务
    int openResult = crypto.open();
    std::cout << "Crypto service open result: " << openResult << std::endl;
    
    // 生成随机数
    uint8_t randomBuf[32];
    int randomResult = crypto.getRandom(randomBuf, 32);
    if (randomResult == 0) {
        std::cout << "Random bytes generated: ";
        for (int i = 0; i < 32; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)randomBuf[i];
        }
        std::cout << std::dec << std::endl;
    }
    
    // SM2密钥对管理
    int keyGenResult = crypto.generateSM2KeyPair(0); // 槽位0
    std::cout << "SM2 key pair generation in slot 0: " << keyGenResult << std::endl;
    
    // 导出公钥
    uint8_t pubKeyBuf[65];
    int exportResult = crypto.exportSM2PubKey(pubKeyBuf, 0);
    if (exportResult == 0) {
        std::cout << "Public key exported from slot 0: ";
        for (int i = 0; i < 10; ++i) { // 只显示前10字节
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)pubKeyBuf[i];
        }
        std::cout << "..." << std::dec << std::endl;
    }
    
    // 用户ID管理
    uint8_t userId[] = "test@example.com";
    int importIdResult = crypto.importID(userId, strlen((char*)userId), 2);
    std::cout << "User ID import result: " << importIdResult << std::endl;
    
    // SM4密钥管理
    uint8_t sm4Key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    int setKeyResult = crypto.setSM4Key(0, sm4Key);
    std::cout << "SM4 key set result: " << setKeyResult << std::endl;
    
    // 关闭加密服务
    int closeResult = crypto.close();
    std::cout << "Crypto service close result: " << closeResult << std::endl;
}

int main() {
    std::cout << "XuanYu CryptoSoftware Demonstration" << std::endl;
    std::cout << "====================================" << std::endl;
    
    std::cout << "Note: This demonstration shows the CryptoSoftware implementation." << std::endl;
    std::cout << "Performance and actual cryptographic strength depend on whether GmSSL is available:" << std::endl;
    std::cout << "- With GmSSL: Real SM2/SM3/SM4 cryptographic operations" << std::endl;
    std::cout << "- Without GmSSL: Simplified placeholder implementations for testing" << std::endl;
    
    try {
        // 演示各种加密功能
        demonstrateSM3();
        demonstrateSM4();
        demonstrateSM2();
        demonstrateICryptoProviderInterface();
        
        std::cout << "\n====================================" << std::endl;
        std::cout << "Demonstration completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during demonstration: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error during demonstration" << std::endl;
        return 1;
    }
    
    return 0;
}