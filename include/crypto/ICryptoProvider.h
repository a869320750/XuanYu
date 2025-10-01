#pragma once

#include <memory>
#include <vector>
#include <string>
#include <cstdint>

namespace xuanyu {
namespace crypto {

/**
 * @brief 加密提供者接口
 * 设计与大唐硬件芯片接口保持一致，采用槽位管理模式
 * 密钥存储在内部槽位中，通过索引号操作，确保软硬件实现的一致性
 */
class ICryptoProvider {
public:
    virtual ~ICryptoProvider() = default;
    
    // ==================== 设备管理 ====================
    
    /**
     * @brief 打开设备/初始化加密库
     * @return 错误代码，0表示成功
     */
    virtual int open() = 0;
    
    /**
     * @brief 关闭设备/清理加密库资源
     * @return 错误代码，0表示成功
     */
    virtual int close() = 0;
    
    // ==================== 随机数生成 ====================
    
    /**
     * @brief 获取随机数
     * @param rndBuf [OUT] 随机数缓冲区
     * @param rndByteLen [IN] 随机数长度
     * @return 错误代码，0表示成功
     */
    virtual int getRandom(uint8_t* rndBuf, uint16_t rndByteLen) = 0;
    
    /**
     * @brief 获取加密随机数（硬件实现返回密文，软件实现可返回明文）
     * @param rndBuf [OUT] 随机数缓冲区
     * @param rndByteLen [IN] 随机数长度
     * @return 错误代码，0表示成功
     */
    virtual int getSecureRandom(uint8_t* rndBuf, uint16_t rndByteLen) = 0;
    
    // ==================== SM2密钥管理 ====================
    
    /**
     * @brief 生成SM2密钥对
     * @param keyPairIndex [IN] 密钥对索引号（0~3）
     * @return 错误代码，0表示成功
     */
    virtual int generateSM2KeyPair(uint8_t keyPairIndex) = 0;
    
    /**
     * @brief 删除SM2密钥对
     * @param keyPairIndex [IN] 密钥对索引号（0~3）
     * @return 错误代码，0表示成功
     */
    virtual int deleteSM2KeyPair(uint8_t keyPairIndex) = 0;
    
    /**
     * @brief 导入SM2密钥对
     * @param priKeyBuf [IN] 私钥数据（32字节）
     * @param pubKeyBuf [IN] 公钥数据（65字节，未压缩格式）
     * @param keyPairIndex [IN] 密钥对索引号（0~3）
     * @return 错误代码，0表示成功
     */
    virtual int importSM2KeyPair(const uint8_t* priKeyBuf, const uint8_t* pubKeyBuf, uint8_t keyPairIndex) = 0;
    
    /**
     * @brief 导入SM2公钥
     * @param pubKeyBuf [IN] 公钥数据（65字节，未压缩格式）
     * @param keyPairIndex [IN] 密钥对索引号（0~3）
     * @return 错误代码，0表示成功
     */
    virtual int importSM2PubKey(const uint8_t* pubKeyBuf, uint8_t keyPairIndex) = 0;
    
    /**
     * @brief 导入SM2私钥
     * @param priKeyBuf [IN] 私钥数据（32字节）
     * @param keyIndex [IN] 私钥索引号（0~3）
     * @return 错误代码，0表示成功
     */
    virtual int importSM2PriKey(const uint8_t* priKeyBuf, uint8_t keyIndex) = 0;
    
    /**
     * @brief 导出SM2公钥
     * @param pubKeyBuf [OUT] 公钥数据缓冲区（65字节）
     * @param keyPairIndex [IN] 密钥对索引号（0~3）
     * @return 错误代码，0表示成功
     */
    virtual int exportSM2PubKey(uint8_t* pubKeyBuf, uint8_t keyPairIndex) = 0;
    
    // ==================== SM2加解密 ====================
    
    /**
     * @brief SM2加密
     * @param cipher [OUT] 密文缓冲区（明文长度+96字节）
     * @param msg [IN] 明文数据
     * @param msgByteLen [IN] 明文长度
     * @param keyPairIndex [IN] 密钥对索引号（0~3）
     * @return 错误代码，0表示成功
     * @note 输出密文格式：C1 || C3 || C2
     */
    virtual int sm2Encrypt(uint8_t* cipher, const uint8_t* msg, uint16_t msgByteLen, uint8_t keyPairIndex) = 0;
    
    /**
     * @brief SM2解密
     * @param msg [OUT] 明文缓冲区（密文长度-96字节）
     * @param cipher [IN] 密文数据
     * @param cipherByteLen [IN] 密文长度
     * @param keyPairIndex [IN] 密钥对索引号（0~3）
     * @return 错误代码，0表示成功
     * @note 输入密文格式：C1 || C3 || C2
     */
    virtual int sm2Decrypt(uint8_t* msg, const uint8_t* cipher, uint16_t cipherByteLen, uint8_t keyPairIndex) = 0;
    
    // ==================== SM2签名验签 ====================
    
    /**
     * @brief SM2签名
     * @param signBuf [OUT] 签名缓冲区（64字节，R||S格式）
     * @param msg [IN] 消息数据
     * @param msgByteLen [IN] 消息长度
     * @param keyPairIndex [IN] 密钥对索引号（0~3）
     * @param idIndex [IN] 用户ID索引号
     * @return 错误代码，0表示成功
     */
    virtual int sm2Sign(uint8_t* signBuf, const uint8_t* msg, uint16_t msgByteLen, uint8_t keyPairIndex, uint8_t idIndex) = 0;
    
    /**
     * @brief SM2验签
     * @param signBuf [IN] 签名数据（64字节，R||S格式）
     * @param msg [IN] 消息数据
     * @param msgByteLen [IN] 消息长度
     * @param keyPairIndex [IN] 密钥对索引号（0~3）
     * @param idIndex [IN] 用户ID索引号
     * @return 错误代码，0表示成功
     */
    virtual int sm2Verify(const uint8_t* signBuf, const uint8_t* msg, uint16_t msgByteLen, uint8_t keyPairIndex, uint8_t idIndex) = 0;
    
    /**
     * @brief SM2签名（摘要模式）
     * @param signBuf [OUT] 签名缓冲区（64字节，R||S格式）
     * @param digest [IN] 摘要数据（32字节）
     * @param keyPairIndex [IN] 密钥对索引号（0~3）
     * @return 错误代码，0表示成功
     */
    virtual int sm2SignDigest(uint8_t* signBuf, const uint8_t* digest, uint8_t keyPairIndex) = 0;
    
    /**
     * @brief SM2验签（摘要模式）
     * @param signBuf [IN] 签名数据（64字节，R||S格式）
     * @param digest [IN] 摘要数据（32字节）
     * @param keyPairIndex [IN] 密钥对索引号（0~3）
     * @return 错误代码，0表示成功
     */
    virtual int sm2VerifyDigest(const uint8_t* signBuf, const uint8_t* digest, uint8_t keyPairIndex) = 0;

    
    // ==================== 用户ID管理 ====================
    
    /**
     * @brief 导入用户ID
     * @param idBuf [IN] ID数据
     * @param idByteLen [IN] ID数据长度（不超过254字节）
     * @param idIndex [IN] ID索引号（2~3）
     * @return 错误代码，0表示成功
     */
    virtual int importID(const uint8_t* idBuf, uint16_t idByteLen, uint8_t idIndex) = 0;
    
    /**
     * @brief 导出用户ID
     * @param idBuf [OUT] ID数据缓冲区
     * @param idByteLen [OUT] ID数据长度
     * @param idIndex [IN] ID索引号（2~3）
     * @return 错误代码，0表示成功
     */
    virtual int exportID(uint8_t* idBuf, uint16_t* idByteLen, uint8_t idIndex) = 0;
    
    // ==================== SM3算法 ====================
    
    /**
     * @brief SM3初始化
     * @return 错误代码，0表示成功
     */
    virtual int sm3Init() = 0;
    
    /**
     * @brief SM3数据更新
     * @param msgBuf [IN] 消息数据
     * @param msgByteLen [IN] 消息长度
     * @return 错误代码，0表示成功
     */
    virtual int sm3Update(const uint8_t* msgBuf, uint16_t msgByteLen) = 0;
    
    /**
     * @brief SM3运算结束
     * @param hashBuf [OUT] 哈希值缓冲区（32字节）
     * @return 错误代码，0表示成功
     */
    virtual int sm3Final(uint8_t* hashBuf) = 0;
    
    /**
     * @brief SM3单块运算
     * @param msgBuf [IN] 消息数据
     * @param msgByteLen [IN] 消息长度
     * @param hashBuf [OUT] 哈希值缓冲区（32字节）
     * @return 错误代码，0表示成功
     */
    virtual int sm3Hash(const uint8_t* msgBuf, uint16_t msgByteLen, uint8_t* hashBuf) = 0;

    
    // ==================== SM4密钥管理 ====================
    
    /**
     * @brief 设置SM4密钥到指定槽位
     * @param keyIndex [IN] 密钥索引号（0~5）
     * @param keyBuf [IN] 密钥数据（16字节）
     * @return 错误代码，0表示成功
     */
    virtual int setSM4Key(uint8_t keyIndex, const uint8_t* keyBuf) = 0;
    
    // ==================== SM4算法 ====================
    
    /**
     * @brief SM4初始化
     * @param keyIndex [IN] 密钥索引号（<6）
     * @param type [IN] 加解密类型（0:加密, 1:解密）
     * @param mode [IN] 运算模式（0:ECB, 1:CBC, 2:CFB, 3:OFB）
     * @param icv [IN] 初始向量（16字节）
     * @return 错误代码，0表示成功
     */
    virtual int sm4Init(uint8_t keyIndex, uint8_t type, uint8_t mode, const uint8_t* icv) = 0;
    
    /**
     * @brief SM4数据更新
     * @param keyIndex [IN] 密钥索引号（<6）
     * @param inputBuf [IN] 输入数据
     * @param msgByteLen [IN] 数据长度（必须为16的整数倍）
     * @param outputBuf [OUT] 输出数据缓冲区
     * @return 错误代码，0表示成功
     */
    virtual int sm4Update(uint8_t keyIndex, const uint8_t* inputBuf, uint16_t msgByteLen, uint8_t* outputBuf) = 0;
    
    /**
     * @brief SM4运算结束
     * @param keyIndex [IN] 密钥索引号（<6）
     * @return 错误代码，0表示成功
     */
    virtual int sm4Final(uint8_t keyIndex) = 0;
    
    /**
     * @brief SM4整块运算
     * @param keyIndex [IN] 密钥索引号（<6）
     * @param type [IN] 加解密类型（0:加密, 1:解密）
     * @param mode [IN] 运算模式（0:ECB, 1:CBC, 2:CFB, 3:OFB）
     * @param icv [IN] 初始向量（16字节）
     * @param inputBuf [IN] 输入数据
     * @param msgByteLen [IN] 数据长度（必须为16的整数倍）
     * @param outputBuf [OUT] 输出数据缓冲区
     * @return 错误代码，0表示成功
     */
    virtual int sm4Crypto(uint8_t keyIndex, uint8_t type, uint8_t mode, const uint8_t* icv, 
                         const uint8_t* inputBuf, uint16_t msgByteLen, uint8_t* outputBuf) = 0;
};

} // namespace crypto
} // namespace xuanyu