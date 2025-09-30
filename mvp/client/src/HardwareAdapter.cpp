#include "HardwareAdapter.h"
#include <cstring>
#include <array>
#include <vector>
#include <cstdint>

extern "C" {
#include "FuncLib.h"
}

using namespace mvp;

int HardwareAdapter::Open() {
    return Dmt_FuncLib_Open();
}

void HardwareAdapter::Close() {
    Dmt_FuncLib_Close();
}

int HardwareAdapter::GetChipFirmwareVersion(unsigned char* buf) {
    return Dmt_Get_ChipFirmwareVersion(buf);
}

int HardwareAdapter::SM3_Hash(const unsigned char* data, int len, unsigned char* out) {
    // SDK expects a writable buffer for some APIs; copy to local if needed.
    // For SM3_Hash the SDK treats input as read-only; forward directly.
    int rc = Dmt_SM3_Hash(const_cast<unsigned char*>(data), len, out);
    return rc;
}

int HardwareAdapter::Dev_Auth() {
    return Dmt_Dev_Auth();
}

int HardwareAdapter::Import_SM4Key(const std::array<unsigned char,16>& key) {
    // Copy to writable C buffer before calling SDK to avoid const_cast on caller data.
    unsigned char keybuf[16];
    std::memcpy(keybuf, key.data(), sizeof(keybuf));

    // Use index 0 by default for now.
    int rc = Dmt_Download_SM4Key(keybuf, 16, 0);
    return rc;
}

/*
mode 参数使用
0 -> ECB 每个 16 字节块独立加密。优点：并行，可随机访问；缺点：相同明文块产出相同密文，容易泄露结构。不推荐用于多块消息。
1 -> CBC 每块先与前一密文异或，再加密。需要 IV（首块），相同明文在不同 IV 下输出不同。能串联依赖，解密可并行。需要对明文做填充（你的 SDK 声明“只支持整块且内部无填充” —— 所以必须在调用前手动做 16 字节对齐/填充）。
2 -> CFB 把前一密文（或 IV）加密后输出与明文异或，得到密文（可做流式）。理论上能处理非整块数据，但你当前硬件要求“整数块”，所以也要对齐。CFB 出错传播比 CBC 小。
3 -> OFB 把前一输出加密后与明文异或，变为流密码；错误不会扩散到后续块。也需要 IV，并且可预生成伪随机流。仍需按硬件要求对齐。
*/
int HardwareAdapter::SM4_Crypto(int keyIndex, unsigned char type, unsigned char mode,
                                unsigned char* icv, const unsigned char* in, int inLen, unsigned char* out) {
    // Copy input to writable buffer if SDK requires non-const input pointer
    unsigned char inbuf[512];
    if (inLen > static_cast<int>(sizeof(inbuf))) {
        return -1;
    }

    std::memcpy(inbuf, in, static_cast<size_t>(inLen));
    int rc = Dmt_SM4_Crypto(keyIndex, type, mode, icv, inbuf, inLen, out);
    return rc;
}


int HardwareAdapter::SM2_GenerateKey(uint8_t slot) {
    // 调用 SDK 生成 SM2 密钥对
    return Dmt_SM2_GenKeyPair(slot);
}

int HardwareAdapter::SM2_ExportPublicKey(uint8_t slot, std::vector<uint8_t>& outPub) {
    unsigned char tmp[512] = {0};
    int rc = Dmt_Export_SM2PubKey(tmp, slot);
    if (rc != RSP_STATUS_OK) {
        return rc;
    }

    // heuristic to determine actual public key length
    size_t len = 0;
    if (tmp[0] == 0x04 && sizeof(tmp) >= 65) {
        len = 65; // uncompressed point: 0x04 || X(32) || Y(32)
    } else if ((tmp[0] == 0x02 || tmp[0] == 0x03) && sizeof(tmp) >= 33) {
        len = 33; // compressed point: 0x02/0x03 || X(32)
    } else {
        // fallback: find last non-zero byte
        for (int i = static_cast<int>(sizeof(tmp)) - 1; i >= 0; --i) {
            if (tmp[i] != 0) {
                len = static_cast<size_t>(i) + 1;
                break;
            }
        }
    }

    if (len == 0 || len > sizeof(tmp)) {
        return -1; // 无法确定长度，返回错误，需进一步排查/确认 SDK 行为
    }

    outPub.assign(tmp, tmp + len);
    return RSP_STATUS_OK;
}

int HardwareAdapter::SM2_ImportPublicKey(uint8_t slot, const std::vector<uint8_t>& pub) {
    // 复制到本地可写缓冲并调用 SDK
    if (pub.empty()) return -1;
    unsigned char buf[512] = {0};
    size_t len = pub.size();
    if (len > sizeof(buf)) return -1;
    std::memcpy(buf, pub.data(), len);
    // SDK 提供单独导入公钥的接口
    return Dmt_Import_SM2PubKey(buf, slot);
}

int HardwareAdapter::SM2_Encrypt(uint8_t slot, const std::vector<uint8_t>& plaintext, std::vector<uint8_t>& outCipher) {
    if (plaintext.empty()) return -1;
    // SDK 说明输出长度为 msgbytelen + 96
    size_t inlen = plaintext.size();
    size_t expect_out = inlen + 96;
    if (expect_out > 65535) return -1; // 超出合理范围
    std::vector<unsigned char> outbuf(expect_out);
    // Dmt_SM2_Encrypt(unsigned char* cipher, unsigned char *msg, unsigned short msgbytelen, unsigned char KeyPairIndex);
    int rc = Dmt_SM2_Encrypt(outbuf.data(), const_cast<unsigned char*>(plaintext.data()), static_cast<unsigned short>(inlen), slot);
    if (rc != RSP_STATUS_OK) return rc;
    outCipher.assign(outbuf.begin(), outbuf.end());
    return RSP_STATUS_OK;
}

int HardwareAdapter::SM2_Decrypt(uint8_t slot, const std::vector<uint8_t>& cipher, std::vector<uint8_t>& outPlain) {
    if (cipher.empty()) return -1;
    size_t cipherlen = cipher.size();
    if (cipherlen < 96) return -1;
    size_t expect_out = cipherlen - 96;
    std::vector<unsigned char> outbuf(expect_out);
    // Dmt_SM2_Decrypt(unsigned char* msg, unsigned char *cipher, unsigned short cipherbytelen, unsigned char KeyPairIndex);
    int rc = Dmt_SM2_Decrypt(outbuf.data(), const_cast<unsigned char*>(cipher.data()), static_cast<unsigned short>(cipherlen), slot);
    if (rc != RSP_STATUS_OK) return rc;
    outPlain.assign(outbuf.begin(), outbuf.end());
    return RSP_STATUS_OK;
}

int HardwareAdapter::SM2_Sign(uint8_t slot, const std::vector<uint8_t>& data, std::vector<uint8_t>& outSig) {
    if (data.empty()) return -1;
    // 输出为 R||S 共 64 字节
    unsigned char sigbuf[64] = {0};
    // Dmt_SM2_Sign(unsigned char* signbuf, unsigned char* msg, unsigned short msgbytelen, unsigned char KeyPairIndex, unsigned char IDIndex);
    int rc = Dmt_SM2_Sign(sigbuf, const_cast<unsigned char*>(data.data()), static_cast<unsigned short>(data.size()), slot, 0);
    if (rc != RSP_STATUS_OK) return rc;
    outSig.assign(sigbuf, sigbuf + 64);
    return RSP_STATUS_OK;
}

int HardwareAdapter::SM2_Verify(const std::vector<uint8_t>& pubkey, const std::vector<uint8_t>& data, const std::vector<uint8_t>& sig) {
    if (pubkey.empty() || data.empty() || sig.empty()) return -1;
    // 复制 pubkey 到可写缓冲（SDK 可能需要）
    unsigned char pubbuf[512] = {0};
    if (pubkey.size() > sizeof(pubbuf)) return -1;
    std::memcpy(pubbuf, pubkey.data(), pubkey.size());
    // SDK 不直接提供“使用外部公钥验签”的原子接口，方案：将公钥导入到一个临时索引（比如 3），然后调用 Dmt_SM2_Verify
    const unsigned char tempSlot = 3;
    int rc = Dmt_Import_SM2PubKey(pubbuf, tempSlot);
    if (rc != RSP_STATUS_OK) return rc;
    // Dmt_SM2_Verify(unsigned char* signbuf, unsigned char* msg, unsigned short msgbytelen, unsigned char KeyPairIndex, unsigned char IDIndex);
    // 注意：signbuf 为输入签名
    unsigned char signbuf_local[128] = {0};
    if (sig.size() > sizeof(signbuf_local)) return -1;
    std::memcpy(signbuf_local, sig.data(), sig.size());
    rc = Dmt_SM2_Verify(signbuf_local, const_cast<unsigned char*>(data.data()), static_cast<unsigned short>(data.size()), tempSlot, 0);
    // 可选：删除临时索引的公钥/密钥对（如果 SDK 支持）
    // Dmt_SM2_DeleteKeyPair(tempSlot);
    return rc;
}