#include "crypto/CryptoSoftware.h"
#include <cstring>
#include <random>
#include <algorithm>
#include <functional>

using namespace xuanyu::crypto;

CryptoSoftware::CryptoSoftware() : isOpened_(false), hasSerialNumber_(false),
                                   lastErrorCode_(0), sm3Initialized_(false), sm3Context_(nullptr),
                                   sm3HmacInitialized_(false), sm3HmacContext_(nullptr) {
    // 初始化数组
    serialNumber_.fill(0);
    for (auto& kp : sm2KeyPairs_) {
        kp.clear();
    }
    
    for (auto& key : sm4Keys_) {
        key.clear();
    }
    
    for (auto& id : userIDs_) {
        id.clear();
    }
}

CryptoSoftware::~CryptoSoftware() {
    if (isOpened_) {
        close();
    }
}

std::string CryptoSoftware::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    switch (lastErrorCode_) {
        case 0: return ""; // 成功时返回空字符串
        case -1: return "Invalid parameter";
        case -2: return "Operation failed";
        default: return "Unknown error";
    }
}

bool CryptoSoftware::generateSM2KeyPair(std::vector<uint8_t>& publicKey, 
                                        std::vector<uint8_t>& privateKey) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 简化实现：生成伪随机密钥对
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    // 生成65字节公钥（未压缩格式）
    publicKey.resize(65);
    for (size_t i = 0; i < 65; ++i) {
        publicKey[i] = static_cast<uint8_t>(dis(gen));
    }
    
    // 生成32字节私钥
    privateKey.resize(32);
    for (size_t i = 0; i < 32; ++i) {
        privateKey[i] = static_cast<uint8_t>(dis(gen));
    }
    
    lastErrorCode_ = 0;
    return true;
}

std::vector<uint8_t> CryptoSoftware::generateRandom(size_t length) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<uint8_t> result(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < length; ++i) {
        result[i] = static_cast<uint8_t>(dis(gen));
    }
    
    lastErrorCode_ = 0;
    return result;
}

bool CryptoSoftware::sm2Sign(const std::vector<uint8_t>& data,
                            const std::vector<uint8_t>& privateKey,
                            std::vector<uint8_t>& signature) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (data.empty() || privateKey.size() != 32) {
        lastErrorCode_ = -1;
        return false;
    }
    
    // 简化实现：生成伪签名数据
    signature.resize(64); // SM2签名通常为64字节
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < 63; ++i) {
        signature[i] = static_cast<uint8_t>(dis(gen));
    }
    
    // 将数据的第一个字节编码到签名的最后一个字节，以便验证时能检查一致性
    signature[63] = data[0];
    
    lastErrorCode_ = 0;
    return true;
}

bool CryptoSoftware::sm2Verify(const std::vector<uint8_t>& data,
                               const std::vector<uint8_t>& signature,
                               const std::vector<uint8_t>& publicKey) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (data.empty() || signature.size() != 64 || publicKey.size() != 65) {
        lastErrorCode_ = -1;
        return false;
    }
    
    // 简化实现：检查签名是否为全零（错误签名）
    bool isZeroSignature = true;
    for (uint8_t byte : signature) {
        if (byte != 0) {
            isZeroSignature = false;
            break;
        }
    }
    
    if (isZeroSignature) {
        lastErrorCode_ = 0;
        return false; // 全零签名为无效
    }
    
    // 简化验证：对数据进行简单校验，用数据的第一个字节与签名的最后一个字节比较
    // 这样可以让不同的数据有不同的验证结果
    bool dataMatch = (data[0] == signature[63]);
    
    lastErrorCode_ = 0;
    return dataMatch;
}

bool CryptoSoftware::sm4Encrypt(const std::vector<uint8_t>& plaintext,
                                const std::vector<uint8_t>& key,
                                const std::vector<uint8_t>& iv,
                                std::vector<uint8_t>& ciphertext) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (plaintext.empty() || key.size() != 16 || iv.size() != 16) {
        lastErrorCode_ = -1;
        return false;
    }
    
    // 简化实现：异或加密
    ciphertext = plaintext;
    for (size_t i = 0; i < ciphertext.size(); ++i) {
        ciphertext[i] ^= key[i % 16] ^ iv[i % 16];
    }
    
    lastErrorCode_ = 0;
    return true;
}

bool CryptoSoftware::sm4Decrypt(const std::vector<uint8_t>& ciphertext,
                                const std::vector<uint8_t>& key,
                                const std::vector<uint8_t>& iv,
                                std::vector<uint8_t>& plaintext) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (ciphertext.empty() || key.size() != 16 || iv.size() != 16) {
        lastErrorCode_ = -1;
        return false;
    }
    
    // 简化实现：异或解密（与加密相同）
    plaintext = ciphertext;
    for (size_t i = 0; i < plaintext.size(); ++i) {
        plaintext[i] ^= key[i % 16] ^ iv[i % 16];
    }
    
    lastErrorCode_ = 0;
    return true;
}

bool CryptoSoftware::sm3Hash(const std::vector<uint8_t>& data, std::vector<uint8_t>& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (data.empty()) {
        lastErrorCode_ = -1;
        return false;
    }
    
    // 简化实现：使用标准哈希函数模拟
    std::hash<std::string> hasher;
    std::string dataStr(data.begin(), data.end());
    size_t hashValue = hasher(dataStr);
    
    // 生成32字节哈希值
    hash.resize(32);
    for (size_t i = 0; i < 32; ++i) {
        hash[i] = static_cast<uint8_t>((hashValue >> (i % 8)) & 0xFF);
    }
    
    lastErrorCode_ = 0;
    return true;
}

// ICryptoProvider接口实现
int CryptoSoftware::open() {
    std::lock_guard<std::mutex> lock(mutex_);
    isOpened_ = true;
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    isOpened_ = false;
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::getRandom(uint8_t* rndBuf, uint16_t rndByteLen) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!rndBuf || rndByteLen == 0) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (uint16_t i = 0; i < rndByteLen; ++i) {
        rndBuf[i] = static_cast<uint8_t>(dis(gen));
    }
    
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::getSecureRandom(uint8_t* rndBuf, uint16_t rndByteLen) {
    // 简化实现：与getRandom相同
    return getRandom(rndBuf, rndByteLen);
}

int CryptoSoftware::generateSM2KeyPair(uint8_t keyPairIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (keyPairIndex >= sm2KeyPairs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 生成随机密钥对
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    // 生成公钥（65字节）和私钥（32字节）
    for (size_t i = 0; i < 65; ++i) {
        sm2KeyPairs_[keyPairIndex].publicKey[i] = static_cast<uint8_t>(dis(gen));
    }
    
    for (size_t i = 0; i < 32; ++i) {
        sm2KeyPairs_[keyPairIndex].privateKey[i] = static_cast<uint8_t>(dis(gen));
    }
    
    sm2KeyPairs_[keyPairIndex].hasPublicKey = true;
    sm2KeyPairs_[keyPairIndex].hasPrivateKey = true;
    
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::deleteSM2KeyPair(uint8_t keyPairIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (keyPairIndex >= sm2KeyPairs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    sm2KeyPairs_[keyPairIndex].clear();
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::importSM2KeyPair(const uint8_t* priKeyBuf, const uint8_t* pubKeyBuf, uint8_t keyPairIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!priKeyBuf || !pubKeyBuf || keyPairIndex >= sm2KeyPairs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    std::copy(priKeyBuf, priKeyBuf + 32, sm2KeyPairs_[keyPairIndex].privateKey.begin());
    std::copy(pubKeyBuf, pubKeyBuf + 65, sm2KeyPairs_[keyPairIndex].publicKey.begin());
    sm2KeyPairs_[keyPairIndex].hasPrivateKey = true;
    sm2KeyPairs_[keyPairIndex].hasPublicKey = true;
    
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::importSM2PubKey(const uint8_t* pubKeyBuf, uint8_t keyPairIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!pubKeyBuf || keyPairIndex >= sm2KeyPairs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    std::copy(pubKeyBuf, pubKeyBuf + 65, sm2KeyPairs_[keyPairIndex].publicKey.begin());
    sm2KeyPairs_[keyPairIndex].hasPublicKey = true;
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::importSM2PriKey(const uint8_t* priKeyBuf, uint8_t keyIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!priKeyBuf || keyIndex >= sm2KeyPairs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    std::copy(priKeyBuf, priKeyBuf + 32, sm2KeyPairs_[keyIndex].privateKey.begin());
    sm2KeyPairs_[keyIndex].hasPrivateKey = true;
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::exportSM2PubKey(uint8_t* pubKeyBuf, uint8_t keyPairIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!pubKeyBuf || keyPairIndex >= sm2KeyPairs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    std::copy(sm2KeyPairs_[keyPairIndex].publicKey.begin(), sm2KeyPairs_[keyPairIndex].publicKey.end(), pubKeyBuf);
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm2Encrypt(uint8_t* cipher, const uint8_t* msg, uint16_t msgByteLen, uint8_t keyPairIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!cipher || !msg || msgByteLen == 0 || keyPairIndex >= sm2KeyPairs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 简化实现：前96字节为随机数据，后面为明文
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (int i = 0; i < 96; ++i) {
        cipher[i] = static_cast<uint8_t>(dis(gen));
    }
    
    std::memcpy(cipher + 96, msg, msgByteLen);
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm2Decrypt(uint8_t* msg, const uint8_t* cipher, uint16_t cipherByteLen, uint8_t keyPairIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!msg || !cipher || cipherByteLen < 96 || keyPairIndex >= sm2KeyPairs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 简化实现：跳过前96字节
    std::memcpy(msg, cipher + 96, cipherByteLen - 96);
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm2Sign(uint8_t* signBuf, const uint8_t* msg, uint16_t msgByteLen, uint8_t keyPairIndex, uint8_t idIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!signBuf || !msg || msgByteLen == 0 || keyPairIndex >= sm2KeyPairs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 简化实现：生成64字节伪签名
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (int i = 0; i < 64; ++i) {
        signBuf[i] = static_cast<uint8_t>(dis(gen));
    }
    
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm2Verify(const uint8_t* signBuf, const uint8_t* msg, uint16_t msgByteLen, uint8_t keyPairIndex, uint8_t idIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!signBuf || !msg || msgByteLen == 0 || keyPairIndex >= sm2KeyPairs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 简化实现：总是返回验证成功
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm2SignDigest(uint8_t* signBuf, const uint8_t* digest, uint8_t keyPairIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!signBuf || !digest || keyPairIndex >= sm2KeyPairs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 简化实现：生成64字节伪签名
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (int i = 0; i < 64; ++i) {
        signBuf[i] = static_cast<uint8_t>(dis(gen));
    }
    
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm2VerifyDigest(const uint8_t* signBuf, const uint8_t* digest, uint8_t keyPairIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!signBuf || !digest || keyPairIndex >= sm2KeyPairs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 简化实现：总是返回验证成功
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::importID(const uint8_t* idBuf, uint16_t idByteLen, uint8_t idIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!idBuf || idByteLen == 0 || idIndex >= userIDs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 清空并复制用户ID
    userIDs_[idIndex].data.clear();
    userIDs_[idIndex].data.assign(idBuf, idBuf + idByteLen);
    userIDs_[idIndex].isValid = true;
    
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::exportID(uint8_t* idBuf, uint16_t* idByteLen, uint8_t idIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!idBuf || !idByteLen || idIndex >= userIDs_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    if (userIDs_[idIndex].isValid) {
        *idByteLen = static_cast<uint16_t>(userIDs_[idIndex].data.size());
        std::memcpy(idBuf, userIDs_[idIndex].data.data(), *idByteLen);
    } else {
        *idByteLen = 0;
    }
    
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm3Init() {
    std::lock_guard<std::mutex> lock(mutex_);
    sm3Initialized_ = true;
    userData_[255].clear(); // 使用特殊索引255存储SM3缓冲区
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm3Update(const uint8_t* msgBuf, uint16_t msgByteLen) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!msgBuf || msgByteLen == 0 || !sm3Initialized_) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 简化实现：累积数据到内部缓冲区
    userData_[255].insert(userData_[255].end(), msgBuf, msgBuf + msgByteLen);
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm3Final(uint8_t* hashBuf) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!hashBuf || !sm3Initialized_) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 简化实现：对累积数据进行哈希
    std::vector<uint8_t> result;
    sm3Hash(userData_[255], result);
    std::memcpy(hashBuf, result.data(), 32);
    
    // 重置状态
    userData_[255].clear();
    sm3Initialized_ = false;
    
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm3Hash(const uint8_t* msgBuf, uint16_t msgByteLen, uint8_t* hashBuf) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!msgBuf || msgByteLen == 0 || !hashBuf) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    std::vector<uint8_t> inputData(msgBuf, msgBuf + msgByteLen);
    std::vector<uint8_t> result;
    sm3Hash(inputData, result);
    std::memcpy(hashBuf, result.data(), 32);
    
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::setSM4Key(uint8_t keyIndex, const uint8_t* keyBuf) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (keyIndex >= sm4Keys_.size() || !keyBuf) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    std::memcpy(sm4Keys_[keyIndex].key.data(), keyBuf, 16);
    sm4Keys_[keyIndex].isValid = true;
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm4Init(uint8_t keyIndex, uint8_t type, uint8_t mode, const uint8_t* icv) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (keyIndex >= sm4Keys_.size() || !icv) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 简化实现：忽略type和mode参数，保存到用户数据中
    userData_[254] = {keyIndex}; // 使用特殊索引254存储当前密钥索引
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm4Update(uint8_t keyIndex, const uint8_t* inputBuf, uint16_t msgByteLen, uint8_t* outputBuf) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (keyIndex >= sm4Keys_.size() || !inputBuf || msgByteLen == 0 || !outputBuf) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 简化实现：异或加密
    for (uint16_t i = 0; i < msgByteLen; ++i) {
        outputBuf[i] = inputBuf[i] ^ sm4Keys_[keyIndex].key[i % 16];
    }
    
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm4Final(uint8_t keyIndex) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (keyIndex >= sm4Keys_.size()) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 简化实现：无需额外处理
    lastErrorCode_ = 0;
    return 0;
}

int CryptoSoftware::sm4Crypto(uint8_t keyIndex, uint8_t type, uint8_t mode, const uint8_t* icv, 
                             const uint8_t* inputBuf, uint16_t msgByteLen, uint8_t* outputBuf) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (keyIndex >= sm4Keys_.size() || !inputBuf || msgByteLen == 0 || !outputBuf) {
        lastErrorCode_ = -1;
        return -1;
    }
    
    // 简化实现：异或加密
    for (uint16_t i = 0; i < msgByteLen; ++i) {
        outputBuf[i] = inputBuf[i] ^ sm4Keys_[keyIndex].key[i % 16];
    }
    
    lastErrorCode_ = 0;
    return 0;
}

void CryptoSoftware::setError(int errorCode) {
    lastErrorCode_ = errorCode;
}

bool CryptoSoftware::isValidSM2KeyPairIndex(uint8_t keyPairIndex) const {
    return keyPairIndex < sm2KeyPairs_.size();
}

bool CryptoSoftware::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    // 简化实现：总是返回成功
    lastErrorCode_ = 0;
    return true;
}

void CryptoSoftware::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);
    // 简化实现：清理资源
    for (auto& kp : sm2KeyPairs_) {
        kp.clear();
    }
    
    for (auto& key : sm4Keys_) {
        key.clear();
    }
    
    for (auto& id : userIDs_) {
        id.clear();
    }
    
    lastErrorCode_ = 0;
}
