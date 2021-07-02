#ifndef _USER_UART__
#define _USER_UART__

#include "stm32f1xx_it.h"     //根据用户MCU进行修改
#include "base.h"

#define uchar    unsigned char
#define uint8    unsigned char
#define uint16   unsigned short int
#define uint32   unsigned long
#define int16    short int
#define int32    long
#define Tx_MAX_SIZE 512   //接收指令最大个数  |  接收指令最大宽度

extern u8 TxBuffer[];

/****************************************************************************
* 名    称： UartInit()
* 功    能： 串口初始化
* 入口参数： 无
* 出口参数： 无
****************************************************************************/
void TFT_Init(void);

/****************************************************************************
* 名    称： Param_Update()
* 功    能： 串口接收数据处理
* 入口参数： 无
* 出口参数： 无
****************************************************************************/
void Param_Update(void);

/*****************************************************************
* 名    称： SendChar()
* 功    能： 发送1个字节
* 入口参数： t  发送的字节
* 出口参数： 无
 *****************************************************************/
void  SendChar(uchar t);

u16 TX_queue_find_cmd(u8 *buffer,u16 buf_len);


#endif
