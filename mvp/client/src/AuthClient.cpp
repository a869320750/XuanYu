#include "AuthClient.h"

#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <random>
#include <sstream>
#include "HardwareAdapter.h"

#include <iomanip>

extern "C" {
#include "FuncLib.h"
}

namespace mvp {

struct AuthClient::Context {
    std::array<char,64> device_id{};
    std::array<char,64> drone_id{};
    std::array<char,128> token{};
    bool is_registered = false;
    bool is_device_open = false;
    int sm4_key_index = 0;
};

static void print_hex(const std::string& prefix, const unsigned char* data, int len) {
    std::cout << prefix;
    for (int i = 0; i < len; ++i) std::printf("%02X ", data[i]);
    std::cout << '\n';
}

static void print_hex_str(const std::string& prefix, const unsigned char* data, int len) {
    std::cout << prefix;
    for (int i = 0; i < len; ++i) std::printf("%02X", data[i]);
    std::cout << '\n';
}

static bool hex_to_bytes(const std::string& hex, unsigned char* out, size_t outlen) {
    if (hex.size() != outlen * 2) return false;
    auto hexval = [](char c)->int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (size_t i = 0; i < outlen; ++i) {
        const int hi = hexval(hex[2*i]);
        const int lo = hexval(hex[2*i+1]);
        if (hi < 0 || lo < 0) return false;
        out[i] = static_cast<unsigned char>((hi << 4) | lo);
    }
    return true;
}

AuthClient::AuthClient(IHardware* hw) : ctx_(std::make_unique<Context>()) {
    if (hw) {
        hw_.reset(hw);
    } else {
        hw_.reset(new HardwareAdapter());
    }
}

AuthClient::~AuthClient() {
    if (ctx_->is_device_open && hw_) hw_->Close();
}

int AuthClient::SM4Import(const std::string& sm4_key) {
    // 仅支持 32 字符十六进制（表示 16 字节）
    if (sm4_key.size() != 32) {
        std::cout << "SM4Import: 需要 32 字符的十六进制字符串（表示16字节密钥）\n";
        return -1;
    }
    std::array<unsigned char,16> key{};
    if (!hex_to_bytes(sm4_key, key.data(), key.size())) {
        std::cout << "SM4Import: 非法的十六进制字符串\n";
        return -1;
    }

    int ret = hw_->Open();
    if (ret != RSP_STATUS_OK) {
        std::cout << "SM4Import: 无法打开设备，错误码 0x" << std::hex << ret << std::dec << "\n";
        return -1;
    }

    ret = hw_->Dev_Auth();
    if (ret != RSP_STATUS_OK) {
        std::cout << "SM4Import: 设备鉴权失败，错误码 0x" << std::hex << ret << std::dec << "\n";
        hw_->Close();
        return -1;
    }

    ret = hw_->Import_SM4Key(key);
    if (ret != RSP_STATUS_OK) {
        std::cout << "SM4Import: 导入密钥失败，错误码 0x" << std::hex << ret << std::dec << "\n";
        hw_->Close();
        return -1;
    }

    ctx_->sm4_key_index = 0; // 目前硬件适配层默认写入索引0
    hw_->Close();
    std::cout << "SM4 key imported\n";
    return 0;
}

int AuthClient::SM4encrypt(const std::string& plaintext) {
    int ret = hw_->Open();
    if (ret != RSP_STATUS_OK) {
        return -1;
    }

    // Prepare input and output buffers
    std::array<unsigned char, 256> input{};
    std::array<unsigned char, 512> output{};
    const int input_len = static_cast<int>(plaintext.size());
    if (input_len > static_cast<int>(input.size())) {
        hw_->Close();
        std::cout << "SM4encrypt: 输入太长\n";
        return -1;
    }
    std::memcpy(input.data(), plaintext.c_str(), static_cast<size_t>(input_len));

    const int key_index = ctx_->sm4_key_index;
    const unsigned char mode = 0;
    const unsigned char type = 0;
    unsigned char* icv = nullptr;
    const int padded_len = ((input_len + 15) / 16) * 16;

    ret = hw_->SM4_Crypto(key_index, type, mode, icv, input.data(), padded_len, output.data());
    if (ret != RSP_STATUS_OK) {
        hw_->Close();
        std::cout << "SM4encrypt: 硬件加密失败 0x" << std::hex << ret << std::dec << "\n";
        return -1;
    }

    print_hex("加密结果: ", output.data(), padded_len);
    print_hex_str("加密结果(十六进制): ", output.data(), padded_len);

    hw_->Close();
    return 0;
}

int AuthClient::SM4decrypt(const std::string& ciphertextHex) {
    const int hex_len = static_cast<int>(ciphertextHex.size());
    if (hex_len % 2 != 0) {
        std::cout << "SM4decrypt: 十六进制字符串长度必须为偶数\n";
        return -1;
    }
    int cipher_len = hex_len / 2;
    if (cipher_len == 0 || cipher_len > 256) {
        std::cout << "SM4decrypt: 密文长度不合法\n";
        return -1;
    }

    std::array<unsigned char,256> ciphertext{};
    if (!hex_to_bytes(ciphertextHex, ciphertext.data(), static_cast<size_t>(cipher_len))) {
        std::cout << "SM4decrypt: 非法的十六进制字符串\n";
        return -1;
    }

    int ret = hw_->Open();
    if (ret != RSP_STATUS_OK) {
        std::cout << "SM4decrypt: 无法打开设备\n";
        return -1;
    }

    const int key_index = ctx_->sm4_key_index;
    const unsigned char mode = 0;
    const unsigned char type = 1;
    unsigned char* icv = nullptr;

    const int padded_len = ((cipher_len + 15) / 16) * 16;
    std::array<unsigned char,256> plaintext{};

    ret = hw_->SM4_Crypto(key_index, type, mode, icv, ciphertext.data(), padded_len, plaintext.data());
    if (ret != RSP_STATUS_OK) {
        hw_->Close();
        std::cout << "SM4decrypt: 硬件解密失败 0x" << std::hex << ret << std::dec << "\n";
        return -1;
    }

    int out_len = cipher_len;
    if (out_len >= static_cast<int>(plaintext.size())) {
        out_len = static_cast<int>(plaintext.size()) - 1;
    }
    plaintext[out_len] = 0;
    std::cout << "解密结果: " << reinterpret_cast<char*>(plaintext.data()) << '\n';

    hw_->Close();
    return 0;
}

// helper hex utils
static bool hex_to_bytes_vec(const std::string& hex, std::vector<uint8_t>& out) {
    if (hex.size() % 2 != 0) return false;
    out.clear();
    out.reserve(hex.size()/2);
    auto val = [](char c)->int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (size_t i = 0; i < hex.size(); i += 2) {
        int hi = val(hex[i]);
        int lo = val(hex[i+1]);
        if (hi < 0 || lo < 0) return false;
        out.push_back(static_cast<uint8_t>((hi << 4) | lo));
    }
    return true;
}

static std::string bytes_to_hex(const std::vector<uint8_t>& v) {
    std::ostringstream os;
    os << std::hex << std::setfill('0');
    for (uint8_t b : v) os << std::setw(2) << (int)b;
    return os.str();
}

// 实现 AuthClient 的高层 SM2 包装（最小实现）
int AuthClient::SM2GenerateKey(uint8_t slot) {
    int rc = hw_->Open();
    if (rc != RSP_STATUS_OK) return -1;
    rc = hw_->Dev_Auth();
    if (rc != RSP_STATUS_OK) { hw_->Close(); return -1; }
    rc = hw_->SM2_GenerateKey(slot);
    hw_->Close();
    return rc;
}

int AuthClient::SM2ExportPublicKeyHex(uint8_t slot, std::string& outPubHex) {
    std::vector<uint8_t> pub;
    int rc = hw_->Open();
    if (rc != RSP_STATUS_OK) return -1;
    rc = hw_->Dev_Auth();
    if (rc != RSP_STATUS_OK) { hw_->Close(); return -1; }
    rc = hw_->SM2_ExportPublicKey(slot, pub);
    hw_->Close();
    if (rc != RSP_STATUS_OK) return rc;
    outPubHex = bytes_to_hex(pub);
    return 0;
}

int AuthClient::SM2ImportPublicKeyHex(uint8_t slot, const std::string& pubHex) {
    std::vector<uint8_t> pub;
    if (!hex_to_bytes_vec(pubHex, pub)) return -1;
    int rc = hw_->Open();
    if (rc != RSP_STATUS_OK) return -1;
    rc = hw_->Dev_Auth();
    if (rc != RSP_STATUS_OK) { hw_->Close(); return -1; }
    rc = hw_->SM2_ImportPublicKey(slot, pub);
    hw_->Close();
    return rc;
}

int AuthClient::SM2EncryptHex(uint8_t slot, const std::string& plaintext, std::string& outCipherHex) {
    std::vector<uint8_t> in(plaintext.begin(), plaintext.end());
    std::vector<uint8_t> out;
    int rc = hw_->Open();
    if (rc != RSP_STATUS_OK) return -1;
    rc = hw_->Dev_Auth();
    if (rc != RSP_STATUS_OK) { hw_->Close(); return -1; }
    rc = hw_->SM2_Encrypt(slot, in, out);
    hw_->Close();
    if (rc != RSP_STATUS_OK) return rc;
    outCipherHex = bytes_to_hex(out);
    return 0;
}

int AuthClient::SM2DecryptHex(uint8_t slot, const std::string& cipherHex, std::string& outPlain) {
    std::vector<uint8_t> cipher;
    if (!hex_to_bytes_vec(cipherHex, cipher)) return -1;
    std::vector<uint8_t> plain;
    int rc = hw_->Open();
    if (rc != RSP_STATUS_OK) return -1;
    rc = hw_->Dev_Auth();
    if (rc != RSP_STATUS_OK) { hw_->Close(); return -1; }
    rc = hw_->SM2_Decrypt(slot, cipher, plain);
    hw_->Close();
    if (rc != RSP_STATUS_OK) return rc;
    outPlain.assign(plain.begin(), plain.end());
    return 0;
}

int AuthClient::SM2SignHex(uint8_t slot, const std::string& data, std::string& outSigHex) {
    std::vector<uint8_t> in(data.begin(), data.end());
    std::vector<uint8_t> sig;
    int rc = hw_->Open();
    if (rc != RSP_STATUS_OK) return -1;
    rc = hw_->Dev_Auth();
    if (rc != RSP_STATUS_OK) { hw_->Close(); return -1; }
    rc = hw_->SM2_Sign(slot, in, sig);
    hw_->Close();
    if (rc != RSP_STATUS_OK) return rc;
    outSigHex = bytes_to_hex(sig);
    return 0;
}

int AuthClient::SM2VerifyHex(const std::string& pubHex, const std::string& data, const std::string& sigHex) {
    std::vector<uint8_t> pub, in, sig;
    if (!hex_to_bytes_vec(pubHex, pub)) return -1;
    in.assign(data.begin(), data.end());
    if (!hex_to_bytes_vec(sigHex, sig)) return -1;
    int rc = hw_->SM2_Verify(pub, in, sig);
    return rc;
}

} // namespace mvp
