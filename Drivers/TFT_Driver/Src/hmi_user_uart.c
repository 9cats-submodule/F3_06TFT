/************************************版权申明********************************************
**                             广州大彩光电科技有限公司
**                             http://www.gz-dc.com
**-----------------------------------文件信息--------------------------------------------
** 文件名称:   hmi_user_uart.c
** 修改时间:   2011-05-18
** 文件说明:   用户MCU串口驱动函数库
** 技术支持：  Tel: 020-82186683  Email: hmi@gz-dc.com Web:www.gz-dc.com
--------------------------------------------------------------------------------------

--------------------------------------------------------------------------------------
                                  使用必读
   hmi_user_uart.c中的串口发送接收函数共3个函数：串口初始化Uartinti()、发送1个字节SendChar()、
   发送字符串SendStrings().若移植到其他平台，需要修改底层寄
   存器设置,但禁止修改函数名称，否则无法与HMI驱动库(hmi_driver.c)匹配。
--------------------------------------------------------------------------------------



----------------------------------------------------------------------------------------
                          1. 基于STM32平台串口驱动
----------------------------------------------------------------------------------------*/
#include "main.h"
#include "hmi_user_uart.h"
#include "hmi_driver.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "base.h"
#include "usart.h"
/****************************************************************************
* 名    称： UartInit()
* 功    能： 串口初始化
* 入口参数： 无
* 出口参数： 无
****************************************************************************/

#define T_UART	huart2
#define R_UART	huart3
//#define R_UART_IRQ	huart1
#define R_UART_	USART3

#define CMD_HEAD 0XEE  //帧头
#define CMD_TAIL 0XFFFCFFFF //帧尾

typedef struct _Tx_QUEUE
{
	u16 _head; //队列头
	u16 _tail;  //队列尾
	u8 _data[Tx_MAX_SIZE];	//队列数据缓存区
}Tx_QUEUE;

u8 RxBuffer;
u8 TxBuffer[Tx_MAX_SIZE];

static Tx_QUEUE Tx_que = {0,0,{0}};  //发送队列
static u32 Tx_state = 0;			 //队列帧尾检测状态
static u8  Tx_pos = 0;				 //当前指令指针位置

void TFT_Init(void)
{
    HAL_UART_Receive_IT(&huart1, &RxBuffer, 1);//使能接收中断
    queue_reset();
}
void Param_Update(void) //获取当前新参数
{
    qsize size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);
    if(size)
    {
        ProcessMessage((PCTRL_MSG)cmd_buffer, size);//指令处理
        LED1_T;
    }
}
/*****************************************************************
* 名    称： SendChar()
* 功    能： 将要传输的数据送入发送队列
* 入口参数： t  发送的字节
* 出口参数： 无
 *****************************************************************/
void Tx_queue_reset(void)
{
	Tx_que._head = Tx_que._tail = 0;
	Tx_pos = Tx_state = 0;
}

void Tx_queue_push(u8 _data)
{
	u16 pos = (Tx_que._head+1)%Tx_MAX_SIZE;
	if(pos != Tx_que._tail)
	{
		Tx_que._data[Tx_que._head] = _data;
		Tx_que._head = pos;
	}
}

static void Tx_queue_pop(u8* _data)
{
	if(Tx_que._tail != Tx_que._head)
	{
		*_data = Tx_que._data[Tx_que._tail];
		Tx_que._tail = (Tx_que._tail+1)%Tx_MAX_SIZE;
	}
}

//获取队列中有效数据个数
static u16 Tx_queue_size()
{
	return ((Tx_que._head+Tx_MAX_SIZE-Tx_que._tail)%Tx_MAX_SIZE);
}

u16 TX_queue_find_cmd(u8 *buffer,u16 buf_len)
{
	u16 cmd_size = 0;
	u8 _data = 0;
	while(Tx_queue_size()>0)
	{
		//取一个数据
		Tx_queue_pop(&_data);

		if(Tx_pos==0&&_data!=CMD_HEAD)//指令第一个字节必须是帧头，否则跳过
		    continue;

		if(Tx_pos<buf_len)//防止缓冲区溢出
			buffer[Tx_pos++] = _data;

		Tx_state = ((Tx_state<<8)|_data);//拼接最后4个字节，组成一个32位整数

		//最后4个字节与帧尾匹配，得到完整帧
		if(Tx_state==CMD_TAIL)
		{
			cmd_size = Tx_pos; //指令字节长度
			Tx_state = 0;  //重新检测帧尾巴
			Tx_pos = 0; //复位指令指针

#if(CRC16_ENABLE)
			//去掉指令头尾EE，尾FFFCFFFF共计5个字节，只计算数据部分CRC
			if(!CheckCRC16(buffer+1,cmd_size-5))//CRC校验
				return 0;

			cmd_size -= 2;//去掉CRC16（2字节）
#endif

			return cmd_size;
		}
	}

	return 0;//没有形成完整的一帧
}

void  SendChar(uchar t)
{
	Tx_queue_push(t);
	/*
	USART1->DR = (t & (uint16_t)0x01FF);
    while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) == RESET);
    while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET);
    */
}


