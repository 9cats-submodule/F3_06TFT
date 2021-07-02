#ifndef _USER_UART__
#define _USER_UART__

#include "stm32f1xx_it.h"     //�����û�MCU�����޸�
#include "base.h"

#define uchar    unsigned char
#define uint8    unsigned char
#define uint16   unsigned short int
#define uint32   unsigned long
#define int16    short int
#define int32    long
#define Tx_MAX_SIZE 512   //����ָ��������  |  ����ָ�������

extern u8 TxBuffer[];

/****************************************************************************
* ��    �ƣ� UartInit()
* ��    �ܣ� ���ڳ�ʼ��
* ��ڲ����� ��
* ���ڲ����� ��
****************************************************************************/
void TFT_Init(void);

/****************************************************************************
* ��    �ƣ� Param_Update()
* ��    �ܣ� ���ڽ������ݴ���
* ��ڲ����� ��
* ���ڲ����� ��
****************************************************************************/
void Param_Update(void);

/*****************************************************************
* ��    �ƣ� SendChar()
* ��    �ܣ� ����1���ֽ�
* ��ڲ����� t  ���͵��ֽ�
* ���ڲ����� ��
 *****************************************************************/
void  SendChar(uchar t);

u16 TX_queue_find_cmd(u8 *buffer,u16 buf_len);


#endif
