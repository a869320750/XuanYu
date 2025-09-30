#pragma once


#include <string>
#include <memory>
#include <array>
#include "IHardware.h"

namespace mvp {

class AuthClient {
public:
    // If hw is nullptr, AuthClient will construct a default real hardware adapter internally.
    explicit AuthClient(IHardware* hw = nullptr);
    ~AuthClient();

    // Non-copyable
    AuthClient(const AuthClient&) = delete;
    AuthClient& operator=(const AuthClient&) = delete;

    // device registration
    int registerDevice(const std::string& deviceId, const std::string& droneId);

    int SM4Import(const std::string& sm4_key_hex);
    int SM4encrypt(const std::string& plaintext);
    int SM4decrypt(const std::string& ciphertextHex);

    // SM2 high-level wrappers (hex/string friendly)
    int SM2GenerateKey(uint8_t slot); // 在槽位生成密钥对
    int SM2ExportPublicKeyHex(uint8_t slot, std::string& outPubHex); // 导出公钥（hex）
    int SM2ImportPublicKeyHex(uint8_t slot, const std::string& pubHex); // 导入公钥（hex）
    int SM2EncryptHex(uint8_t slot, const std::string& plaintext, std::string& outCipherHex); // 用槽位公钥加密，输出 hex
    int SM2DecryptHex(uint8_t slot, const std::string& cipherHex, std::string& outPlain); // 用槽位私钥解密
    int SM2SignHex(uint8_t slot, const std::string& data, std::string& outSigHex); // 签名 -> hex
    int SM2VerifyHex(const std::string& pubHex, const std::string& data, const std::string& sigHex); // 验签

private:
    struct Context;
    std::unique_ptr<Context> ctx_;
    std::unique_ptr<IHardware> hw_; // owned by AuthClient
};

} // namespace mvp
