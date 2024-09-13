#ifndef _SERIAL_H
#define _SERIAL_H
#include "stdio.h"
//uint8_t Serial_RxPacket[4];

void Serial_Init(void);
void Serial_USART1_SendByte(uint8_t Byte);
void Serial_USART2_SendByte(uint8_t Byte);
void Serial_USART3_SendByte(uint8_t Byte);
#endif
