/************************************��Ȩ����********************************************
**                             ���ݴ�ʹ��Ƽ����޹�˾
**                             http://www.gz-dc.com
**-----------------------------------�ļ���Ϣ--------------------------------------------
** �ļ�����:   hmi_user_uart.c
** �޸�ʱ��:   2011-05-18
** �ļ�˵��:   �û�MCU��������������
** ����֧�֣�  Tel: 020-82186683  Email: hmi@gz-dc.com Web:www.gz-dc.com
--------------------------------------------------------------------------------------

--------------------------------------------------------------------------------------
                                  ʹ�ñض�
   hmi_user_uart.c�еĴ��ڷ��ͽ��պ�����3�����������ڳ�ʼ��Uartinti()������1���ֽ�SendChar()��
   �����ַ���SendStrings().����ֲ������ƽ̨����Ҫ�޸ĵײ��
   ��������,����ֹ�޸ĺ������ƣ������޷���HMI������(hmi_driver.c)ƥ�䡣
--------------------------------------------------------------------------------------



----------------------------------------------------------------------------------------
                          1. ����STM32ƽ̨��������
----------------------------------------------------------------------------------------*/
#include "main.h"
#include "hmi_user_uart.h"
#include "hmi_driver.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "base.h"
#include "usart.h"
/****************************************************************************
* ��    �ƣ� UartInit()
* ��    �ܣ� ���ڳ�ʼ��
* ��ڲ����� ��
* ���ڲ����� ��
****************************************************************************/

#define T_UART	huart2
#define R_UART	huart3
//#define R_UART_IRQ	huart1
#define R_UART_	USART3

#define CMD_HEAD 0XEE  //֡ͷ
#define CMD_TAIL 0XFFFCFFFF //֡β

typedef struct _Tx_QUEUE
{
	u16 _head; //����ͷ
	u16 _tail;  //����β
	u8 _data[Tx_MAX_SIZE];	//�������ݻ�����
}Tx_QUEUE;

u8 RxBuffer;
u8 TxBuffer[Tx_MAX_SIZE];

static Tx_QUEUE Tx_que = {0,0,{0}};  //���Ͷ���
static u32 Tx_state = 0;			 //����֡β���״̬
static u8  Tx_pos = 0;				 //��ǰָ��ָ��λ��

void TFT_Init(void)
{
    HAL_UART_Receive_IT(&huart1, &RxBuffer, 1);//ʹ�ܽ����ж�
    queue_reset();
}
void Param_Update(void) //��ȡ��ǰ�²���
{
    qsize size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);
    if(size)
    {
        ProcessMessage((PCTRL_MSG)cmd_buffer, size);//ָ���
        LED1_T;
    }
}
/*****************************************************************
* ��    �ƣ� SendChar()
* ��    �ܣ� ��Ҫ������������뷢�Ͷ���
* ��ڲ����� t  ���͵��ֽ�
* ���ڲ����� ��
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

//��ȡ��������Ч���ݸ���
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
		//ȡһ������
		Tx_queue_pop(&_data);

		if(Tx_pos==0&&_data!=CMD_HEAD)//ָ���һ���ֽڱ�����֡ͷ����������
		    continue;

		if(Tx_pos<buf_len)//��ֹ���������
			buffer[Tx_pos++] = _data;

		Tx_state = ((Tx_state<<8)|_data);//ƴ�����4���ֽڣ����һ��32λ����

		//���4���ֽ���֡βƥ�䣬�õ�����֡
		if(Tx_state==CMD_TAIL)
		{
			cmd_size = Tx_pos; //ָ���ֽڳ���
			Tx_state = 0;  //���¼��֡β��
			Tx_pos = 0; //��λָ��ָ��

#if(CRC16_ENABLE)
			//ȥ��ָ��ͷβEE��βFFFCFFFF����5���ֽڣ�ֻ�������ݲ���CRC
			if(!CheckCRC16(buffer+1,cmd_size-5))//CRCУ��
				return 0;

			cmd_size -= 2;//ȥ��CRC16��2�ֽڣ�
#endif

			return cmd_size;
		}
	}

	return 0;//û���γ�������һ֡
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


