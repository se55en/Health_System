#include "stm32f10x.h"                  // Device header

void LED_PC13_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
void LED_PC13_On(void)
{
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
}
void LED_PC13_Off(void)
{
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
}
