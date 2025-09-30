#ifndef __FUNCLIB_H__
#define __FUNCLIB_H__

#define PKT_ERROR_HEAD								0x8F01
#define PKT_ERROR_CMD								0x8F02
#define PKT_ERROR_PARA								0x8F03
#define PKT_ERROR_LEN								0x8F04
#define PKT_ERROR_XOR								0x8F05
#define PKT_ERROR_CRC								0x8F06

#define	RSP_STATUS_OK								0x0000		//状态正常                                                                   
#define	RSP_ERROR_GET_SN							0x6F01		//获取SN错误
#define	RSP_ERROR_GET_RANDOM						0x6F02		//获取随机数错误
#define RSP_ERROR_GET_VERSION         				0x6F05		//获取版本号失败错误

#define RSP_ERROR_SM2_GEN_KEYPAIR					0x6F06		//SM2密钥对生成错误
#define RSP_ERROR_SM2_DELETE_KEYPAIR				0x6F07		//SM2密钥对销毁错误
#define RSP_ERROR_SM2_IMPORT_PUBKEY                 0x6F08		//SM2导入公钥错误
#define RSP_ERROR_SM2_EXPORT_PUBKEY                 0x6F09		//SM2导出公钥错误
#define RSP_ERROR_SM2_IMPORT_ID		                0x6F0E		//SM2导入ID错误
#define RSP_ERROR_SM2_EXPORT_ID		                0x6F0F		//SM2导出ID错误
#define RSP_ERROR_SM4_CRYPTO         				0x6F11		//SM4加解密运算错误
#define RSP_ERROR_SM2_KEYEXCHANGE					0x6F13		//SM2密钥协商错误

#define	RSP_ERROR_FLASH_PROGRAM						0x6C80		//写FLASH失败错误
#define RSP_ERROR_MEMSPACE_READ_DATA				0x6C81		//数据存储区读数据错误	
#define RSP_ERROR_COM_SEND_FAILED					0x6C82		//端口发送通讯失败
#define RSP_ERROR_COM_RECV_FAILED					0x6C83		//端口接收通讯失败
#define RSP_ERROR_MEMORY_MALLOC						0x6C84		//空间分配失败
#define RSP_ERROR_INPUT_PARA						0x6C85		//函数输入参数错误
#define RSP_ERROR_WAIT_COMPLETE						0x6C86		//等待安全芯片处理错误
#define RSP_DEV_AUTH_ERROR                          0x6C89      //认证失败
#define RSP_NO_PERMISSION_ERROR                     0x6C8A      //权限无效
#define RSP_INIT_ACC_ERROR_S                        0x6C8B      //C0A9认证主失败
#define RSP_INIT_ACC_ERROR_M                        0x6C8C      //主认证C0A9失败

/************************************ function ******************************************/
/*
*	打开设备
*   备注：在使用函数库之前，必须先调用该函数进行设备初始化;
*/
int Dmt_FuncLib_Open(void);

/*
*	关闭设备
*   备注：在设备不使用时，可以调用该函数来关闭设备以节省资源;
*/
int Dmt_FuncLib_Close(void);

/*
*	导入指定索引号的SM2公钥
*	Pubkeybuf		[IN]  指向待导入SM2公钥数据的地址指针
*	Index			[IN]  SM2密钥对索引号
*/
int Dmt_Import_SM2PubKey(unsigned char* Pubkeybuf, unsigned char Index);

/*
*	获取芯片固件发布版本号
*	ChipFirmwareVersion			[OUT]	指向存储固件版本号的地址指针(4字节)
*	备注：返回的固件发布版本号例如: v1.0
*/
int Dmt_Get_ChipFirmwareVersion(unsigned char *ChipFirmwareVersion);

/*
*	获取芯片固件编译版本号
*	ChipFirmwareBuild			[OUT]	指向存储固件编译版本号的地址指针(6字节)
*	备注：返回的固件编译版本号例如: 01.0.1
*/
int Dmt_Get_ChipFirmwareBuild(unsigned char *ChipFirmwareBuild);

/*
*	获取SDK版本号
*	SDKVersion			[OUT]	指向存储SDK版本号的地址指针(4字节)
*	备注：返回的SDK版本号例如: V1.0
*/
int Dmt_Get_SDK_Version(unsigned char *SDKVersion);

/*
*	设备鉴权认证
*/
int Dmt_Dev_Auth(void);
/*
*	导入sn
*	sn			[IN]  指向输入sn数据的地址指针
*	len		    [IN]  sn数据的字节长度
*   备注：导入sn信息的字节长度为32字节;
*/
int Dmt_Import_SN(unsigned char *sn,unsigned int len);
/*
*	导入sm4密钥
*	SM4_Key		[IN] 指向存储SM4密钥数据的地址指针
*/
int Dmt_Import_SM4Key(unsigned char *SM4_Key);
/*
*	导入指定索引号的SM2私钥和公钥
*	Prikeybuf		[IN]  指向待导入SM2私钥数据的地址指针
*	Pubkeybuf		[IN]  指向待导入SM2公钥数据的地址指针
*	Index			[IN]  SM2密钥对索引号
*/
int Dmt_Import_SM2KeyPair(unsigned char* Prikeybuf, unsigned char* Pubkeybuf, unsigned char Index);
/*
*	设置初次鉴权失败限制处理
*	limit			[IN]  允许的鉴权失败次数
*   备注：1、limit默认值为3。
*         2、0表示不启动鉴权失败后的操作；
*         3、其他值表示累计多少次后启动C0A9芯片鉴权失败后的处置动作
*/
int Dmt_Set_InitAcc_Limit(unsigned char limit);



/*
*	导出指定索引号的SM2公钥密文
*	Pubkeybuf		[OUT] 指向存储SM2公钥数据的地址指针
*	Index			[IN]  SM2密钥对索引号
*   备注：调用该函数前需要确保公钥已被提前导入过，否则返回的数据无效;
*/
int Dmt_Export_SM2SPubKey(unsigned char* Pubkeybuf, unsigned char Index);

/*
*	主控芯片鉴权认证安全芯片（C0A9）
*	outbuf 		[OUT] 安全芯片输出的密文数据
*	SR1			[IN]  主芯片生成的随机数R1用SM2公钥加密后的密文
*   SR1_len     [IN]  R1的密文数据，长度为112字节（R1长度+96）
*/
int Dmt_InitAcc_Auth1(unsigned char *SR1,unsigned int SR1_len,unsigned char *outbuf); 
/*
*	安全芯片（C0A9）鉴权认证主控
*	inbuf 		[IN] 主控发送给安全芯片的密文数据
*	inlen		[IN] 输入数据长度
*/
int Dmt_InitAcc_Auth2(unsigned char *inbuf,unsigned int inlen);
/*
*	主控芯片认证安全芯片（C0A9）失败时通知安全芯片
*   当Dmt_InitAcc_Auth1解密后比对不通过时使用
*/
int Dmt_InitAcc_Auth3(void);
/*
*	获取随机数数据密文
*	rndbuf			[OUT] 指向存储随机数数据的地址指针
*	rndbytelen		[IN]  待获取随机数的字节长度
*/
int Dmt_Get_SRandom(unsigned char* rndbuf, unsigned short rndbytelen);
/*
*	过程认证
*	inbuf 		[IN] 主控发送给安全芯片的密文数据
*	inlen		[IN] 输入数据长度   
*   outbuf      [OUT]安全芯片输出的密文数据 
*/
int Dmt_Process_Acc(unsigned char *inbuf,unsigned int inlen,unsigned char *outbuf);

/*
*	导入指定索引号的Userdata
*	idbuf			[IN]  指向输入Userdata数据的地址指针
*	idbytelen		[IN]  Userdata数据的字节长度
*	IDIndex			[IN]  Userdata索引号
*   备注：导入Userdata信息的字节长度不能超过512字节;
*/
int Dmt_Import_Userdata(unsigned char* idbuf, unsigned short idbytelen, unsigned char IDIndex);

/*
*	导出指定索引号的Userdata
*	idbuf			[OUT] 指向存储Userdata数据的地址指针
*	idbytelen		[IN] 导出的Userdata数据的字节长度
*	IDIndex			[IN]  Userdata索引号
*   备注：调用该函数前需要确保Userdata已被提前导入过，否则返回的数据无效;
*/
int Dmt_Export_Userdata(unsigned char* idbuf, unsigned short idbytelen, unsigned char IDIndex);

/*
*	获取芯片SN数据
*	snbuf			[OUT] 指向存储SN数据的地址指针(32字节)
*/
int Dmt_Get_SN(unsigned char* snbuf);

/*
*	获取随机数数据
*	rndbuf			[OUT] 指向存储随机数数据的地址指针
*	rndbytelen		[IN]  待获取随机数的字节长度
*/
int Dmt_Get_Random(unsigned char* rndbuf, unsigned short rndbytelen);
/*
*	生成指定索引号的SM2密钥对
*	KeyPairIndex	[IN]  SM2密钥对索引号(索引号编码为0~3)
*/
int Dmt_SM2_GenKeyPair(unsigned char KeyPairIndex);

/*
*	删除指定索引号的SM2密钥对
*	KeyPairIndex	[IN]  SM2密钥对索引号(索引号编码为0~3)
*/
int Dmt_SM2_DeleteKeyPair(unsigned char KeyPairIndex);
/*
*	导入指定索引号的SM2公钥
*	Pubkeybuf		[IN]  指向待导入SM2公钥数据的地址指针
*	KeyPairIndex	[IN]  SM2密钥对索引号(索引号编码为0~3)
*/
int Dmt_Import_SM2PubKey(unsigned char* Pubkeybuf, unsigned char KeyPairIndex);

/*
*	导入指定索引号的SM2私钥
*	Prikeybuf		[IN]  指向待导入SM2私钥数据的地址指针
*	KeyIndex	[IN]  SM2私钥索引号(索引号编码为0~3)
*/
int Dmt_Import_SM2PriKey(unsigned char* Prikeybuf, unsigned char KeyIndex);

/*
*	导出指定索引号的SM2公钥
*	Pubkeybuf		[OUT] 指向存储SM2公钥数据的地址指针
*	KeyPairIndex	[IN]  SM2密钥对索引号(索引号编码为0~3)
*   备注：调用该函数前需要确保公钥已被提前导入过，否则返回的数据无效;
*/
int Dmt_Export_SM2PubKey(unsigned char* Pubkeybuf, unsigned char KeyPairIndex);

/*
*	使用指定索引号对应的SM2公钥进行加密运算
*	cipher			[OUT] 指向存储SM2密文数据的地址指针
*	msg				[IN]  指向输入SM2明文数据的地址指针
*	msgbytelen		[IN]  SM2明文数据的字节长度
*	KeyPairIndex	[IN]  SM2密钥对索引号(索引号编码为0~3)
*	备注: 1)输出密文结果顺序为: C1 || C3 || C2;
*		  2)加密运算结果的字节长度为msgbytelen + 96;
*/
int Dmt_SM2_Encrypt(unsigned char* cipher, unsigned char *msg, unsigned short msgbytelen, unsigned char KeyPairIndex);

/*
*	使用指定索引号对应的SM2私钥进行解密运算
*	msg				[OUT] 指向存储SM2明文数据的地址指针
*	cipher			[IN]  指向输入SM2密文数据的地址指针
*	cipherbytelen	[IN]  SM2密文数据的字节长度
*	KeyPairIndex	[IN]  SM2密钥对索引号(索引号编码为0~3)
*	备注: 1)输入密文结果顺序为: C1 || C3 || C2;
*		  2)解密运算结果的字节长度为cipherbytelen - 96;
*/
int Dmt_SM2_Decrypt(unsigned char* msg, unsigned char *cipher, unsigned short cipherbytelen, unsigned char KeyPairIndex);

/*
*	使用指定索引号对应的SM2私钥进行签名运算
*	signbuf			[OUT] 指向存储SM2签名结果的地址指针
*	msg				[IN]  指向输入SM2明文消息数据的地址指针
*	msgbytelen		[IN]  SM2消息数据的字节长度
*	KeyPairIndex	[IN]  SM2密钥对索引号(索引号编码为0~3)
*	IDIndex			[IN]  用户id索引号
*	备注: 1)调用该函数之前，需要确保SM2的私钥、公钥及用户ID信息已经提前存储在安全芯片内;
*         2)输出的SM2签名结果为R||S，共64byte；
*/
int Dmt_SM2_Sign(unsigned char* signbuf, unsigned char* msg, unsigned short msgbytelen, unsigned char KeyPairIndex, unsigned char IDIndex);

/*
*	使用指定索引号对应的SM2公钥进行验签运算
*	signbuf			[IN]  指向输入SM2签名结果的地址指针
*	msg				[IN]  指向输入SM2明文消息数据的地址指针
*	msgbytelen		[IN]  SM2消息数据的字节长度
*	KeyPairIndex	[IN]  SM2密钥对索引号(索引号编码为0~3)
*	IDIndex			[IN]  用户id索引号
*	备注: 1)调用该函数前，需要先确保已生成或导入过SM2公钥以及导入过用户id数据;
*/
int Dmt_SM2_Verify(unsigned char* signbuf, unsigned char* msg, unsigned short msgbytelen, unsigned char KeyPairIndex, unsigned char IDIndex);

/*
*	使用指定索引号对应的SM2私钥进行签名运算(不包括预处理过程)
*	signbuf			[OUT] 指向存储SM2签名结果的地址指针
*	digest			[IN]  指向输入摘要数据的地址指针
*	KeyPairIndex	[IN]  SM2密钥对索引号(索引号编码为0~3)
*	备注: 1)调用该函数之前，需要确保SM2的私钥已经提前存储在安全芯片内;
*         2)输出的SM2签名结果为R||S，共64byte；
*/
int Dmt_SM2_Sign_Digest(unsigned char* signbuf, unsigned char* digest, unsigned char KeyPairIndex);

/*
*	使用指定索引号对应的SM2公钥进行验签运算(不包括预处理过程)
*	signbuf			[IN]  指向输入SM2签名结果的地址指针
*	digest			[IN]  指向输入摘要数据的地址指针
*	KeyPairIndex	[IN]  SM2密钥对索引号(索引号编码为0~3)
*	备注: 1)调用该函数前，需要先确保已生成或导入过SM2公钥;
*/
int Dmt_SM2_Verify_Digest(unsigned char* signbuf, unsigned char* digest, unsigned char KeyPairIndex);

/*
*	使用指定的密钥和ID索引号进行密钥协商运算
*	pAgreedKey				[OUT] 指向存储密钥协商结果的地址指针
*	AgreedKeyByteLen		[IN]  密钥协商字节长度
*	SelfKeypairIndex		[IN]  已方SM2密钥对索引号(索引号编码为0~3)
*	SelfTempKeypairIndex	[IN]  已方临时SM2密钥对索引号(索引号编码为0~3)
*	SelfIDIndex				[IN]  已方ID信息索引号(索引号编码为2~3)
*	OtherKeypairIndex		[IN]  对方SM2公钥索引号(索引号编码为0~3)
*	OtherTempKeypairIndex	[IN]  对方临时SM2公钥索引号(索引号编码为0~3)
*	OtherIDIndex			[IN]  对方ID信息索引号(索引号编码为2~3)
*	Mode					[IN]  参数配置 (0: 发起方	1: 响应方)
*	备注: 1)调用该函数之前，需要确保已方SM2公私密钥对、已方用户ID信息、对方公钥数据、对方临时公钥数据、对方用户ID信息已经提前存储在安全芯片内;
*/
int Dmt_SM2_KeyExchange(unsigned char* pAgreedKey, unsigned char AgreedKeyByteLen, unsigned char SelfKeypairIndex, unsigned char SelfTempKeypairIndex, unsigned char SelfIDIndex, unsigned char OtherKeypairIndex, unsigned char OtherTempKeypairIndex, unsigned char OtherIDIndex, unsigned char Mode);

/*
*	导入指定索引号的用户id
*	idbuf			[IN]  指向输入用户id数据的地址指针
*	idbytelen		[IN]  用户id数据的字节长度
*	IDIndex			[IN]  用户id索引号(2~3)
*   备注：导入用户ID信息的字节长度不能超过254字节;
*/
int Dmt_Import_ID(unsigned char* idbuf, unsigned short idbytelen, unsigned char IDIndex);

/*
*	导出指定索引号的用户id
*	idbuf			[OUT] 指向存储用户id数据的地址指针
*	idbytelen		[OUT] 导出的用户id数据的字节长度
*	IDIndex			[IN]  用户id索引号(2~3)
*   备注：调用该函数前需要确保ID信息已被提前导入过，否则返回的数据无效;
*/
int Dmt_Export_ID(unsigned char* idbuf, unsigned short *idbytelen, unsigned char IDIndex);

/*
*	SM3运算初始化
*/
int Dmt_SM3_Init(void);

/*
*	SM3运算数据更新
*	msgbuf			[IN]  指向输入消息数据的地址指针
*	msgbytelen		[IN]  消息数据的字节长度
*	备注: 1)调用该函数前，需要先调用Dmt_SM3_Init函数进行初始化;
*/
int Dmt_SM3_Update(unsigned char *msgbuf, unsigned short msgbytelen);

/*
*	SM3运算结束
*	hashbuf			[OUT] 指向存储哈希结果的地址指针(32字节)
*	备注: 1)调用该函数前，需要先调用Dmt_SM3_Init和Dmt_SM3_Update函数进行初始化及数据更新;
*/
int Dmt_SM3_Final(unsigned char *hashbuf);

/*
*	单块SM3运算
*	msgbuf				[IN]	指向消息数据的地址指针
*	msgbytelen			[IN]	消息数据的字节长度
*	hashbuf				[OUT]	指向存储哈希数据的地址指针(32字节)
*/
int Dmt_SM3_Hash(unsigned char *msgbuf, unsigned short msgbytelen, unsigned char *hashbuf);

/*
*	SM3-HMAC运算初始化
*	InputKey			[IN]	指向输入密钥数据的地址指针
*	KeyByteLen			[IN]	密钥数据的字节长度
*/
int Dmt_SM3_HMAC_Init(unsigned char *InputKey, unsigned short KeyByteLen);

/*
*	SM3-HMAC运算数据更新
*	msgbuf			[IN]  指向输入消息数据的地址指针
*	msgbytelen		[IN]  消息数据的字节长度
*	备注: 1)调用该函数前，需要先调用Dmt_SM3_HMAC_Init函数进行初始化;
*/
int Dmt_SM3_HMAC_Update(unsigned char *msgbuf, unsigned short msgbytelen);

/*
*	SM3-HMAC运算结束
*	hmacbuf			[OUT] 指向存储HMAC结果的地址指针(32字节)
*	备注: 1)调用该函数前，需要先调用Dmt_SM3_HMAC_Init和Dmt_SM3_HMAC_Update函数进行初始化及数据更新;
*/
int Dmt_SM3_HMAC_Final(unsigned char * hmacbuf);

/*
*	单块SM3-HMAC运算
*	InputKey	[IN]	指向密钥数据的地址指针
*	KeyByteLen	[IN]	密钥数据的字节长度
*	msgbuf		[IN]	指向消息数据的地址指针
*	msgbytelen	[IN]	消息数据的字节长度
*	hmacbuf		[OUT]	指向存储MAC数据的地址指针(32字节)
*/
int Dmt_SM3_Hmac(unsigned char *InputKey, unsigned short KeyByteLen, unsigned char *msgbuf, unsigned short msgbytelen, unsigned char *hmacbuf);

/*
*	导入指定索引号的SM4 Key
*	idbuf			[IN]  指向输入Key数据的地址指针
*	idbytelen		[IN]  Keyd数据的字节长度
*	IDIndex			[IN]  Key索引号
*   备注：导入Key信息的字节长度为16字节;
*/
int Dmt_Download_SM4Key(unsigned char* idbuf, unsigned short idbytelen, unsigned char IDIndex);

/*
*	设置SM4算法密钥，并存储在安全芯片内
*	KeyIndex		[IN]	密钥索引号选择(小于6)
*	Mkey			[IN]	指向输入报文密钥数据的地址指针（16字节）
*	KeyType			[IN]	密钥类型选择(0: SM1		1: SM4)
*/
int Dmt_Set_SymmetryMKey(unsigned char KeyIndex, unsigned char* Mkey, unsigned char KeyType);

/*
*	SM4算法运算初始化
*	KeyIndex		[IN]	密钥索引号选择(小于6)
*	type			[IN]	加解密类型(0：表示加密运算；1：表示解密运算)
*	mode			[IN]	运算模式选择(0：表示ECB模式；1：表示CBC模式；2：表示CFB模式；3：表示OFB模式)
*	icv				[IN]	指向输入初始向量数据的地址指针（16字节）
*	备注：1)只支持整数块运算，内部无填充操作。
*		  2)调用该函数前，需要先调用Dmt_Set_SymmetryMKey函数进行设置密钥;
*/
int Dmt_SM4_Init(unsigned char KeyIndex, unsigned char type, unsigned char mode, unsigned char *icv);

/*
*	SM4算法运算数据更新
*	KeyIndex		[IN]	密钥索引号选择(小于6)
*	inputbuf		[IN]	指向输入数据的地址指针
*	msgbytelen		[IN]	输入数据的字节长度(必须为16的整数倍)
*	outputbuf		[OUT]	指向存储输出数据的地址指针
*	备注：1)只支持整数块运算，内部无填充操作。
*		  2)调用该函数前，需要先调用Dmt_SM4_Init函数进行运算类型及模式设置;
*/
int Dmt_SM4_Update(unsigned char KeyIndex, unsigned char *inputbuf, unsigned short msgbytelen, unsigned char *outputbuf);

/*
*	SM4算法运算结束，清除内部缓存
*	KeyIndex		[IN]	密钥索引号选择(小于6)
*	备注：只支持整数块运算，内部无填充操作。
*/
int Dmt_SM4_Final(unsigned char KeyIndex);

/*
*	SM4算法整块运算
*	KeyIndex		[IN]	密钥索引号选择(小于6)
*	type			[IN]	加解密类型(0：表示加密运算；1：表示解密运算)
*	mode			[IN]	运算模式选择(0：表示ECB模式；1：表示CBC模式；2：表示CFB模式；3：表示OFB模式)
*	icv				[IN]	指向输入初始向量数据的地址指针（16字节）
*	inputbuf		[IN]	指向输入数据的地址指针
*	msgbytelen		[IN]	输入数据的字节长度(必须为16的整数倍)
*	outputbuf		[OUT]	指向存储输出数据的地址指针
*	备注：只支持整数块运算，内部无填充操作。
*/
int Dmt_SM4_Crypto(unsigned char KeyIndex, unsigned char type, unsigned char mode, unsigned char *icv, unsigned char *inputbuf, unsigned short msgbytelen, unsigned char *outputbuf);

#endif
