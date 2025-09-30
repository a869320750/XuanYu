#include "AuthClient.h"
#include "HardwareAdapter.h"

#include <iostream>
#include <string>
#include <vector>

using namespace mvp;

static void print_usage(const char* prog_name) {
    std::cout << "用法:\n";
    std::cout << "  " << prog_name << " sm4 importKey <sm4_key_hex>   - 导入 SM4 密钥（32 hex 字符）\n";
    std::cout << "  " << prog_name << " sm4 encrypt <plaintext>       - 使用 SM4 加密\n";
    std::cout << "  " << prog_name << " sm4 decrypt <ciphertext_hex>  - 使用 SM4 解密（输入为 hex）\n\n";

    std::cout << "  " << prog_name << " sm3 hash <message>            - 计算 SM3 哈希（若实现）\n\n";

    std::cout << "  " << prog_name << " sm2 genkey                    - 在设备上生成 SM2 密钥对（若实现）\n";
    std::cout << "  " << prog_name << " sm2 keyex <params...>         - 执行 SM2 密钥协商（若实现）\n";
    std::cout << "\n示例 (# 表示注释，不要在命令行中输入 # 及其后的内容):\n";
    std::cout << "  " << prog_name << " sm4 importKey 00112233445566778899AABBCCDDEEFF  # 导入 SM4 密钥\n";
    std::cout << "  " << prog_name << " sm4 encrypt \"hello world\"                   # 加密明文（注意需按 16 字节填充策略）\n";
    std::cout << "  " << prog_name << " sm4 decrypt 0011AABB...                     # 解密十六进制密文\n";
    std::cout << "  " << prog_name << " sm2 genkey                                # 在槽位 0 生成密钥对（默认）\n";
    std::cout << "  " << prog_name << " sm2 export 0                              # 导出槽位 0 的公钥（hex）\n";
    std::cout << "  " << prog_name << " sm2 import 0 <pubkey_hex>                 # 将外部公钥导入到槽位 0\n";
    std::cout << "  " << prog_name << " sm2 encrypt 0 \"message\"                 # 使用槽位 0 的公钥加密，输出 hex\n";
    std::cout << "  " << prog_name << " sm2 decrypt 0 <cipher_hex>                # 使用槽位 0 的私钥解密\n";
    std::cout << "  " << prog_name << " sm2 sign 0 \"message\"                    # 使用槽位 0 的私钥签名，输出 hex\n";
    std::cout << "  " << prog_name << " sm2 verify <pub_hex> \"message\" <sig_hex>  # 使用公钥验签\n";
}

int main(int argc, char* argv[]) {
    std::cout << "UAVChip-Auth MVP CLI\n";
    std::cout << "=====================\n\n";

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    // 把 argv 转为更安全的 C++ 容器
    std::vector<std::string> args;
    args.reserve(argc - 1);
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);

    // args[0] = top-level command, args[1] = subcommand (if present)
    if (args.size() < 2) {
        print_usage(argv[0]);
        return 1;
    }

    AuthClient client;
    const std::string &cmd = args[0];

    if (cmd == "sm4") {
        const std::string &sub = args[1];
        if (sub == "importKey" || sub == "import") {
            if (args.size() != 2 + 1) { print_usage(argv[0]); return 1; }
            return client.SM4Import(args[2]);
        } else if (sub == "encrypt") {
            if (args.size() != 2 + 1) { print_usage(argv[0]); return 1; }
            return client.SM4encrypt(args[2]);
        } else if (sub == "decrypt") {
            if (args.size() != 2 + 1) { print_usage(argv[0]); return 1; }
            return client.SM4decrypt(args[2]);
        } else {
            std::cout << "未知 sm4 子命令: " << sub << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    if (cmd == "sm3") {
        const std::string &sub = args[1];
        if (sub == "hash") {
            if (args.size() != 2 + 1) { print_usage(argv[0]); return 1; }
            const std::string& message = args[2];
            HardwareAdapter hw;
            int rc = hw.Open();
            if (rc != 0) { std::cout << "无法打开设备: 0x" << std::hex << rc << std::dec << "\n"; return 1; }
            rc = hw.Dev_Auth();
            if (rc != 0) { std::cout << "设备鉴权失败: 0x" << std::hex << rc << std::dec << "\n"; hw.Close(); return 1; }
            unsigned char hash[32] = {0};
            rc = hw.SM3_Hash(reinterpret_cast<const unsigned char*>(message.data()), static_cast<int>(message.size()), hash);
            if (rc != 0) { std::cout << "SM3 计算失败: 0x" << std::hex << rc << std::dec << "\n"; hw.Close(); return 1; }
            std::cout << "SM3(" << message << ") = ";
            for (int i = 0; i < 32; ++i) std::printf("%02X", hash[i]);
            std::cout << "\n";
            hw.Close();
            return 0;
        } else {
            std::cout << "未知 sm3 子命令: " << sub << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    if (cmd == "sm2") {
        const std::string &sub = args[1];
        if (sub == "genkey") {
            AuthClient client;
            int rc = client.SM2GenerateKey(0);
            if (rc == 0) std::cout << "SM2 key generated in slot 0\n";
            else std::cout << "SM2 genkey failed: 0x" << std::hex << rc << std::dec << "\n";
            return rc == 0 ? 0 : 1;
        } else if (sub == "export") {
            if (args.size() != 2 + 1) { print_usage(argv[0]); return 1; }
            int slot = 0;
            try { slot = std::stoi(args[2]); } catch(...) { std::cout << "slot 必须是数字\n"; return 1; }
            AuthClient client;
            std::string pubHex;
            int rc = client.SM2ExportPublicKeyHex(static_cast<uint8_t>(slot), pubHex);
            if (rc != 0) { std::cout << "导出公钥失败: 0x" << std::hex << rc << std::dec << "\n"; return 1; }
            std::cout << "pubkey(hex): " << pubHex << "\n";
            return 0;
        } else if (sub == "import") {
            if (args.size() != 2 + 2) { print_usage(argv[0]); return 1; }
            int slot = 0;
            try { slot = std::stoi(args[2]); } catch(...) { std::cout << "slot 必须是数字\n"; return 1; }
            const std::string& pubHex = args[3];
            AuthClient client;
            int rc = client.SM2ImportPublicKeyHex(static_cast<uint8_t>(slot), pubHex);
            if (rc != 0) { std::cout << "导入公钥失败: 0x" << std::hex << rc << std::dec << "\n"; return 1; }
            std::cout << "公钥已导入槽位 " << slot << "\n";
            return 0;
        } else if (sub == "encrypt") {
            if (args.size() != 2 + 2) { print_usage(argv[0]); return 1; }
            int slot = 0;
            try { slot = std::stoi(args[2]); } catch(...) { std::cout << "slot 必须是数字\n"; return 1; }
            const std::string& plaintext = args[3];
            AuthClient client;
            std::string cipherHex;
            int rc = client.SM2EncryptHex(static_cast<uint8_t>(slot), plaintext, cipherHex);
            if (rc != 0) { std::cout << "SM2 encrypt 失败: 0x" << std::hex << rc << std::dec << "\n"; return 1; }
            std::cout << "cipher(hex): " << cipherHex << "\n";
            return 0;
        } else if (sub == "decrypt") {
            if (args.size() != 2 + 2) { print_usage(argv[0]); return 1; }
            int slot = 0;
            try { slot = std::stoi(args[2]); } catch(...) { std::cout << "slot 必须是数字\n"; return 1; }
            const std::string& cipherHex = args[3];
            AuthClient client;
            std::string plain;
            int rc = client.SM2DecryptHex(static_cast<uint8_t>(slot), cipherHex, plain);
            if (rc != 0) { std::cout << "SM2 decrypt 失败: 0x" << std::hex << rc << std::dec << "\n"; return 1; }
            std::cout << "plain: " << plain << "\n";
            return 0;
        } else if (sub == "sign") {
            if (args.size() != 2 + 2) { print_usage(argv[0]); return 1; }
            int slot = 0;
            try { slot = std::stoi(args[2]); } catch(...) { std::cout << "slot 必须是数字\n"; return 1; }
            const std::string& data = args[3];
            AuthClient client;
            std::string sigHex;
            int rc = client.SM2SignHex(static_cast<uint8_t>(slot), data, sigHex);
            if (rc != 0) { std::cout << "SM2 sign 失败: 0x" << std::hex << rc << std::dec << "\n"; return 1; }
            std::cout << "sig(hex): " << sigHex << "\n";
            return 0;
        } else if (sub == "verify") {
            // expect: sm2 verify <pub_hex> <message> <sig_hex>
            if (args.size() != 3 + 2) { print_usage(argv[0]); return 1; }
            const std::string& pubHex = args[2];
            const std::string& data = args[3];
            const std::string& sigHex = args[4];
            AuthClient client;
            int rc = client.SM2VerifyHex(pubHex, data, sigHex);
            if (rc == 0) { std::cout << "验签成功\n"; return 0; }
            std::cout << "验签失败: 0x" << std::hex << rc << std::dec << "\n";
            return 1;
        } else if (sub == "keyex") {
            std::cout << "SM2 key exchange 未实现（占位），请使用 AuthClient 的 SM2KeyExchange 方法（若实现）。\n";
            return 1;
        } else {
            std::cout << "未知 sm2 子命令: " << sub << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    std::cout << "未知命令: " << cmd << "\n";
    print_usage(argv[0]);
    return 1;
}