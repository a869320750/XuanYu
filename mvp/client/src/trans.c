#include "trans.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_DEV_PATH "/dev/i2c-4"
#define I2C_ADDR 0x38

static int i2c_fd = -1;

// 简单的延时函数实现
void delay_ms(int ms) {
    usleep(ms * 1000); // usleep接受微秒参数
    // printf("[TRANS] delay_ms: %d ms\n", ms);
}

/*
*	申请指定大小的动态内存
*	size			[IN]  待申请动态内存的字节大小
*/
void *Dmt_Malloc(unsigned int size)
{
    void *ptr = malloc(size);
    // printf("[TRANS] Dmt_Malloc: size=%u, ptr=%p\n", size, ptr);
    return ptr;
}

/*
*	释放指定位置的动态内存
*	ptr				[IN]  指向待释放动态内存的地址指针
*/
void Dmt_Free(void *ptr)
{
    // printf("[TRANS] Dmt_Free: ptr=%p\n", ptr);
    free(ptr);
}

/*
*	打开设备
*   备注：1)该函数由FuncLib.h中的Dmt_FuncLib_Open接口来主动调用;
*         2)该函数主要用于初始化主控芯片的通讯接口，若通讯接口已提前初始化，则该函数可为空;
*/
int Dmt_Device_Open(void)
{
    // printf("[TRANS] Dmt_Device_Open: open %s\n", I2C_DEV_PATH);
    i2c_fd = open(I2C_DEV_PATH, O_RDWR);
    if (i2c_fd < 0) {
        perror("[TRANS] open");
        return -1;
    }
    if (ioctl(i2c_fd, I2C_SLAVE, I2C_ADDR) < 0) {
        perror("[TRANS] ioctl I2C_SLAVE");
        close(i2c_fd);
        i2c_fd = -1;
        return -2;
    }
    // printf("[TRANS] Dmt_Device_Open: success, fd=%d\n", i2c_fd);
    return 0;
}

/*
*	关闭设备
*   备注：1)该函数由FuncLib.h中的Dmt_FuncLib_Close接口来主动调用;
*         2)该函数主要用于初始化主控芯片的通讯接口，若通讯接口已提前初始化，则该函数可为空;
*/
int Dmt_Device_Close(void)
{
    // printf("[TRANS] Dmt_Device_Close: fd=%d\n", i2c_fd);
    if (i2c_fd >= 0) {
        close(i2c_fd);
        i2c_fd = -1;
        // printf("[TRANS] Dmt_Device_Close: closed\n");
        return 0;
    }
    // printf("[TRANS] Dmt_Device_Close: not opened\n");
    return -1;
}

/*
*	发送数据
*	buf				[IN]  指向待发送数据的地址指针
*	size			[IN]  待发送数据的字节长度
*	备注：1)该函数由FuncLib.h中的功能接口来主动调用;
*         2)返回0表示主控芯片发送命令数据成功;
*/
int Dmt_Send_Data(unsigned char *buf, unsigned int size)
{
    // printf("[TRANS] Dmt_Send_Data: fd=%d, size=%u\n", i2c_fd, size);
    if (i2c_fd < 0) {
        // printf("[TRANS] Dmt_Send_Data: device not opened\n");
        return -1;
    }
    int ret = write(i2c_fd, buf, size);
    if (ret != (int)size) {
        perror("[TRANS] write");
        // printf("[TRANS] Dmt_Send_Data: write failed, ret=%d\n", ret);
        return -2;
    }
    // printf("[TRANS] Dmt_Send_Data: write success\n");
    return 0;
}

/*
*	接收数据
*	buf				[OUT] 指向存储接收数据的地址指针
*	size			[IN]  待接收数据的字节长度
*	备注：1)该函数由FuncLib.h中的功能接口来主动调用;
*         2)返回0表示主控芯片接收响应数据成功;
*/
int Dmt_Recv_Data(unsigned char *buf, unsigned int size)
{
    // printf("[TRANS] Dmt_Recv_Data: fd=%d, size=%u\n", i2c_fd, size);
    if (i2c_fd < 0) {
        // printf("[TRANS] Dmt_Recv_Data: device not opened\n");
        return -1;
    }
    int ret = read(i2c_fd, buf, size);
    if (ret != (int)size) {
        perror("[TRANS] read");
        // printf("[TRANS] Dmt_Recv_Data: read failed, ret=%d\n", ret);
        return -2;
    }
    // printf("[TRANS] Dmt_Recv_Data: read success, data:");
    for (unsigned int i = 0; i < size; ++i) {
        // printf(" %02X", buf[i]);
    }
    // printf("\n");
    return 0;
}

/*
*	等待安全芯片处理完成
*	备注：1)该函数由FuncLib.h中的功能接口来主动调用;
*         2)返回0表示安全芯片处理完成,主控芯片可以读取响应结果,
*           返回非0表示安全芯片还在处理，主控需要再等次等待;
*/
int Dmt_Waiting_Complete(void)
{
    // printf("[TRANS] Dmt_Waiting_Complete: delay 50ms\n");
    delay_ms(50);
    // printf("[TRANS] Dmt_Waiting_Complete: done\n");
    return 0;
}
