#include "stm32f10x.h"                  // Device header

void PC13LED_Init(void)
{	//初始化外部时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	//创建结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//初始化GPIO口
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//熄灭LED
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void PC13LED_On(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

void PC13LED_Off(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}
