#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_Init(void)
{
	PWM_Init();
}

void Servo_SetAngle(float Angle)
{
	PWM_SetCompare2(Angle / 270 * 2000 + 500);
}

void Servo_Stop(void)
{
	PWM_SetCompare2(0);
}

