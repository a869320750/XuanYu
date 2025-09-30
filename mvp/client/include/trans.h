#ifndef __TRANS_H__
#define __TRANS_H__

/*
*	申请指定大小的动态内存
*	size			[IN]  待申请动态内存的字节大小
*/
void *Dmt_Malloc(unsigned int size);

/*
*	释放指定位置的动态内存
*	ptr				[IN]  指向待释放动态内存的地址指针
*/
void Dmt_Free(void *ptr);

/*
*	打开设备
*   备注：1)该函数由FuncLib.h中的Dmt_FuncLib_Open接口来主动调用;
*         2)该函数主要用于初始化主控芯片的通讯接口，若通讯接口已提前初始化，则该函数可为空;
*/
int Dmt_Device_Open(void);

/*
*	关闭设备
*   备注：1)该函数由FuncLib.h中的Dmt_FuncLib_Close接口来主动调用;
*         2)该函数主要用于初始化主控芯片的通讯接口，若通讯接口已提前初始化，则该函数可为空;
*/
int Dmt_Device_Close(void);

/*
*	发送数据
*	buf				[IN]  指向待发送数据的地址指针
*	size			[IN]  待发送数据的字节长度
*	备注：1)该函数由FuncLib.h中的功能接口来主动调用;
*         2)返回0表示主控芯片发送命令数据成功;
*/
int Dmt_Send_Data(unsigned char *buf, unsigned int size);

/*
*	接收数据
*	buf				[OUT] 指向存储接收数据的地址指针
*	size			[IN]  待接收数据的字节长度
*	备注：1)该函数由FuncLib.h中的功能接口来主动调用;
*         2)返回0表示主控芯片接收响应数据成功;
*/
int Dmt_Recv_Data(unsigned char *buf, unsigned int size);

/*
*	等待安全芯片处理完成
*	备注：1)该函数由FuncLib.h中的功能接口来主动调用;
*         2)返回0表示安全芯片处理完成,主控芯片可以读取响应结果；
*/
int Dmt_Waiting_Complete(void);

#endif
