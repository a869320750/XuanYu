#include "crypto/CryptoSoftware.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <memory>

using namespace std::chrono;
using namespace xuanyu::crypto;

class CryptoBenchmark {
private:
    std::unique_ptr<CryptoSoftware> crypto;
    
public:
    CryptoBenchmark() {
        crypto = std::make_unique<CryptoSoftware>();
    }
    
    void runAllBenchmarks() {
        std::cout << "=== CryptoSoftware Performance Benchmark ===" << std::endl;
        
        benchmarkSM3();
        std::cout << std::endl;
        
        benchmarkSM4();
        std::cout << std::endl;
        
        benchmarkSM2();
        std::cout << std::endl;
        
        benchmarkRandomGeneration();
    }
    
private:
    void benchmarkSM3() {
        std::cout << "--- SM3 Hash Benchmark ---" << std::endl;
        
        std::vector<size_t> dataSizes = {32, 64, 128, 256, 512, 1024, 4096};
        
        for (size_t size : dataSizes) {
            std::vector<uint8_t> data(size, 0xAA);
            const int iterations = (size <= 1024) ? 1000 : 100;
            
            auto start = high_resolution_clock::now();
            
            for (int i = 0; i < iterations; ++i) {
                std::vector<uint8_t> hash;
                crypto->sm3Hash(data, hash);
            }
            
            auto end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            
            double avgTime = (double)duration.count() / iterations;
            double throughput = (double)size * iterations / duration.count(); // MB/s
            
            std::cout << std::setw(6) << size << " bytes: " 
                      << std::setw(8) << std::fixed << std::setprecision(2) << avgTime << " μs/op, "
                      << std::setw(8) << std::fixed << std::setprecision(2) << throughput << " MB/s"
                      << std::endl;
        }
    }
    
    void benchmarkSM4() {
        std::cout << "--- SM4 Encryption Benchmark ---" << std::endl;
        
        std::vector<size_t> dataSizes = {16, 64, 256, 1024, 4096};
        std::vector<uint8_t> key(16, 0x42);
        std::vector<uint8_t> iv(16, 0x00);
        
        for (size_t size : dataSizes) {
            std::vector<uint8_t> data(size, 0xAA);
            const int iterations = (size <= 1024) ? 500 : 50;
            
            auto start = high_resolution_clock::now();
            
            for (int i = 0; i < iterations; ++i) {
                std::vector<uint8_t> ciphertext;
                crypto->sm4Encrypt(data, key, iv, ciphertext);
            }
            
            auto end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            
            double avgTime = (double)duration.count() / iterations;
            double throughput = (double)size * iterations / duration.count(); // MB/s
            
            std::cout << std::setw(6) << size << " bytes: " 
                      << std::setw(8) << std::fixed << std::setprecision(2) << avgTime << " μs/op, "
                      << std::setw(8) << std::fixed << std::setprecision(2) << throughput << " MB/s"
                      << std::endl;
        }
    }
    
    void benchmarkSM2() {
        std::cout << "--- SM2 Signature Benchmark ---" << std::endl;
        
        std::vector<uint8_t> publicKey, privateKey;
        bool keyGenResult = crypto->generateSM2KeyPair(publicKey, privateKey);
        
        if (!keyGenResult) {
            std::cout << "Key generation failed, skipping SM2 benchmark" << std::endl;
            return;
        }
        
        std::vector<uint8_t> data(32, 0xAA); // 32字节测试数据
        const int iterations = 100;
        
        // 签名性能测试
        auto start = high_resolution_clock::now();
        
        std::vector<std::vector<uint8_t>> signatures;
        for (int i = 0; i < iterations; ++i) {
            std::vector<uint8_t> signature;
            if (crypto->sm2Sign(data, privateKey, signature)) {
                signatures.push_back(signature);
            }
        }
        
        auto end = high_resolution_clock::now();
        auto signDuration = duration_cast<microseconds>(end - start);
        
        double avgSignTime = (double)signDuration.count() / signatures.size();
        
        std::cout << "Signature: " << std::setw(8) << std::fixed << std::setprecision(2) 
                  << avgSignTime << " μs/op (" << signatures.size() << " operations)" << std::endl;
        
        // 验签性能测试
        if (!signatures.empty()) {
            start = high_resolution_clock::now();
            
            int verifyCount = 0;
            for (const auto& sig : signatures) {
                if (crypto->sm2Verify(data, sig, publicKey)) {
                    verifyCount++;
                }
            }
            
            end = high_resolution_clock::now();
            auto verifyDuration = duration_cast<microseconds>(end - start);
            
            double avgVerifyTime = (double)verifyDuration.count() / verifyCount;
            
            std::cout << "Verify:    " << std::setw(8) << std::fixed << std::setprecision(2) 
                      << avgVerifyTime << " μs/op (" << verifyCount << " operations)" << std::endl;
        }
    }
    
    void benchmarkRandomGeneration() {
        std::cout << "--- Random Generation Benchmark ---" << std::endl;
        
        std::vector<size_t> sizes = {16, 32, 64, 128, 256};
        
        for (size_t size : sizes) {
            const int iterations = 1000;
            
            auto start = high_resolution_clock::now();
            
            for (int i = 0; i < iterations; ++i) {
                auto random = crypto->generateRandom(size);
                // 防止编译器优化掉
                volatile uint8_t dummy = random[0];
                (void)dummy;
            }
            
            auto end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            
            double avgTime = (double)duration.count() / iterations;
            double throughput = (double)size * iterations / duration.count(); // MB/s
            
            std::cout << std::setw(6) << size << " bytes: " 
                      << std::setw(8) << std::fixed << std::setprecision(2) << avgTime << " μs/op, "
                      << std::setw(8) << std::fixed << std::setprecision(2) << throughput << " MB/s"
                      << std::endl;
        }
    }
};

int main() {
    std::cout << "Starting CryptoSoftware Performance Benchmark..." << std::endl;
    std::cout << "Note: Performance may vary depending on whether GmSSL is available." << std::endl;
    std::cout << "With GmSSL: Real cryptographic operations" << std::endl;
    std::cout << "Without GmSSL: Simplified placeholder implementations" << std::endl;
    std::cout << std::endl;
    
    CryptoBenchmark benchmark;
    benchmark.runAllBenchmarks();
    
    std::cout << std::endl;
    std::cout << "Benchmark completed!" << std::endl;
    
    return 0;
}