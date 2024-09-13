#include "stm32f10x.h"                  // Device header
#include "Serial.h"
#include "string.h"
#include "Servo.h"
#include "Timer.h"
#include "Delay.h"
#include "MYRTC.h"
//#include "Key.h"
//测试
#include "PC13LED.h"


#define medicine_num 4//四个药盒
#define heat_time 10

uint8_t RxData;//USART1接收的数据
uint8_t RxData2;//USART2接收的数据
uint8_t RxState = 0;//接收状态
uint8_t pRxPacket = 0;//接收数据位置指针
uint8_t RxFlag = 0;//接收标志位
uint8_t CloseFlag = 0;//关闭药盒标志
char Serial_RxPacket[100];//接收数据数组

uint8_t hour = 9;//时
uint8_t min = 59;//分
uint8_t sec = 40;//秒

uint8_t medicine_flag = 0;//比较过时间药品的数目

typedef struct{
	uint8_t clock_hour;	//时（闹钟设定时间）
	uint8_t clock_min;	//分
	uint8_t clock_sec;	//秒
	uint8_t clock_status;//闹钟状态，0为关闭，1为开启
	uint8_t eat_flag;	//吃药标志，0为不用吃，1为要吃
	uint8_t heat_flag;  //加热标志，1为加热
}medicine_t;

medicine_t medicine[medicine_num] = {
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0}
};
typedef struct{
	float  	Temperature;        //红外检测的温度
    int 	HeartRate;          //心率
    int 	BloodOxygen;      //血氧
    float  	Weight;           //体重kg
    int 	Height;             //身高cm
    float 	BMI;              //BMI值
}data_t;
data_t Index_Data;

void Data_Init(void)
{
	Index_Data.Temperature = 20.3;
	Index_Data.HeartRate = 83;
	Index_Data.BloodOxygen = 99;
	Index_Data.Weight = 75;
	Index_Data.Height = 178;
	Index_Data.BMI = 19.9;
}
void Deal_Data(char* str)
{
	char* p = NULL;
	//心率显示
	p = strstr(str, "H");
	if(p != NULL)
	{
		Index_Data.HeartRate = (p[1]-'0')*10 + (p[2]-'0');
		printf("data.heart.txt=\"%d\"\xff\xff\xff", Index_Data.HeartRate);
	}
	//血氧显示
	p = strstr(str, "O");
	if(p != NULL)
	{
		Index_Data.BloodOxygen = (p[1]-'0')*10 + (p[2]-'0');
		printf("data.blood.txt=\"%d\"\xff\xff\xff", Index_Data.BloodOxygen);
	}
	//身高显示
	p = strstr(str, "G");
	if(p != NULL)
	{
		Index_Data.Height = (p[1]-'0')*100 + (p[2]-'0')*10 + (p[3]-'0');
		printf("data.height.txt=\"%dcm\"\xff\xff\xff", Index_Data.Height);
	}
	//温度显示
	p = strstr(str, "T");
	if(p != NULL)
	{
		Index_Data.Temperature = (p[1]-'0')*10 + (p[2]-'0') + (p[4]-'0')*0.1;
		printf("data.temperature.txt=\"%.1f C\"\xff\xff\xff", Index_Data.Temperature);
	}
	//体重
	p = strstr(str, "W");
	if(p != NULL)
	{
		Index_Data.Weight = (p[1]-'0')*10 + (p[2]-'0') + (p[4]-'0')*0.1;
		printf("data.weight.txt=\"%.1fkg\"\xff\xff\xff", Index_Data.Weight);
	}
	//身高
	p = strstr(str, "B");
	if(p != NULL)
	{
		Index_Data.BMI = (p[1]-'0')*10 + (p[2]-'0') + (p[4]-'0')*0.1;
		printf("data.bmi.txt=\"%.1f\"\xff\xff\xff", Index_Data.BMI);
	}
	
}
void Judge(void)
{	
	int JudgeFlag = 0;//判断标志，如果都正常就为0
	if(Index_Data.Temperature > 38){Serial_USART2_SendByte(0x02);Delay_s(2);JudgeFlag = 1;}
	else if(Index_Data.Temperature < 16){Serial_USART2_SendByte(0x03);Delay_s(2);JudgeFlag = 1;}
	
	if(Index_Data.BMI > 23.9){Serial_USART2_SendByte(0x04);Delay_s(2); JudgeFlag = 1;}
	else if(Index_Data.BMI < 18.5){Serial_USART2_SendByte(0x05);Delay_s(2);JudgeFlag = 1;}
	
	if(Index_Data.HeartRate > 100){Serial_USART2_SendByte(0x06);Delay_s(2); JudgeFlag = 1;}
	else if(Index_Data.HeartRate < 55){Serial_USART2_SendByte(0x07);Delay_s(2);JudgeFlag = 1;}
	
	if(Index_Data.BloodOxygen < 95){Serial_USART2_SendByte(0x08);Delay_s(2); JudgeFlag = 1;}

	if(JudgeFlag == 0)//都正常
	{
		Serial_USART2_SendByte(0x09);
	}
}
void Open_Box(void)
{
	uint8_t flag = 0xb1; 
	for(int i = 0; i < medicine_num; i++)
	{
		if(flag + i == RxData2)
		{
			Servo_SetAngle((i+1)*60);
		}
	}
	if(RxData2 == 0xa3)
	{
		Servo_SetAngle(0);
	}
}
void ReadMessage(void)//读取接收的信息
{
	if(RxFlag == 1)//判断接收的消息
	{
		uint8_t num;//第几个药盒
		if(Serial_RxPacket[0] > '9')
		{
			Deal_Data(Serial_RxPacket);
		}
		else
		{
			num = Serial_RxPacket[0] - '0';
			medicine[num].clock_hour = (Serial_RxPacket[2]-'0')*10 + Serial_RxPacket[3]-'0';
			medicine[num].clock_min = (Serial_RxPacket[5]-'0')*10 + Serial_RxPacket[6]-'0';
			medicine[num].clock_sec = (Serial_RxPacket[8]-'0')*10 + Serial_RxPacket[9]-'0';
			medicine[num].clock_status = 1;//开启时钟
		}
		RxFlag = 0;
	}
}

void Compare_Time(void)
{
	uint8_t h_t, m_t;//判断是否开始加热，吃药前三分钟开始加热
	for(int i = 0; i < medicine_num; i++)
	{
		medicine_flag++;
		if(hour == medicine[i].clock_hour && min == medicine[i].clock_min && 
			sec == medicine[i].clock_sec && medicine[i].clock_status == 1)
		{
			medicine[i].eat_flag = 1;//标记为要吃药
		}
		if(medicine[i].clock_min < heat_time)
		{
			m_t = medicine[i].clock_min+60-heat_time;
			if(medicine[i].clock_hour <= 0)
			{
				h_t = medicine[i].clock_hour + 24 -1;
			}
			else{
				h_t = medicine[i].clock_hour - 1;
			}
		}
		else{
			m_t = medicine[i].clock_min-heat_time;
			h_t = medicine[i].clock_hour;
		}
		if(hour == h_t && min == m_t && medicine[i].clock_status == 1 && medicine[i].heat_flag == 0)
		{
			medicine[i].heat_flag = 1;
			Serial_USART1_SendByte('1');//给3861发消息开始加热
		}
	}
}

void Check_Eat(void)//查看吃药状态
{
	for(int i = 0; i < medicine_num; i++)
	{
		if(medicine[i].eat_flag == 1)
		{

			Serial_USART2_SendByte(0x01);//给语音发消息(语音输出提醒吃药)
			Servo_SetAngle((i+1)*60);
			while(1)
			{
				if(CloseFlag == 1)
				{
					Servo_SetAngle(0);//关闭药盒
					medicine[i].eat_flag = 0;
					CloseFlag = 0;
					Serial_USART1_SendByte('0');//给3861发消息停止加热
					medicine[i].heat_flag = 0;
					break;
				}
			}
			
		}
	}
}
//****************************************************************主函数**********************************************************
int main(void)
{
	PC13LED_Init();
	
	MyRTC_Init();
	MyRTC_ReadTime();
	hour = MyRTC_Time[3];min = MyRTC_Time[4];sec = MyRTC_Time[5];//设置时间
	
	Data_Init();
	Serial_Init();
	Servo_Init();
	Timer_Init();
	Delay_ms(500);
	printf("start.date.txt=\"%d-%d-%d\"\xff\xff\xff", MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
	
	Servo_SetAngle(0);
	while(1)
	{
		ReadMessage();
		Compare_Time();
		if(medicine_flag >= medicine_num)//每个药盒都比较时间过一次
		{
			Check_Eat();
			medicine_flag = 0;
		}
	}
}



void USART1_IRQHandler(void)//串口接收信息中断（接收3861信息）
{

	if(USART_GetFlagStatus(USART1, USART_IT_RXNE) == SET)
	{
		RxData = USART_ReceiveData(USART1);
		if(RxState == 0)
		{
			if(RxData == '@' && RxFlag == 0)
			{
				RxState = 1;
				pRxPacket = 0;
			}
		}
		else if(RxState == 1)
		{
			if(RxData == '#')//判断是不是包尾
			{
				RxState = 2;
			}
			else//不是包尾读取数据
			{
				Serial_RxPacket[pRxPacket] = RxData;
				pRxPacket++;
			}
		}
		else if(RxState == 2)
		{
			if(RxData == '#')
			{
				Serial_RxPacket[pRxPacket] = '\0';
				RxState = 0;
				RxFlag = 1;
			}
		}
	}
}
void USART2_IRQHandler(void)//串口2接收信息中断（接收语音模块信息）
{
	if(USART_GetFlagStatus(USART2, USART_IT_RXNE) == SET)
	{
		RxData2 = USART_ReceiveData(USART2);
		if(RxData2 == 0xa1)
		{
			CloseFlag = 1;
		}
		else if(RxData2 == 0xa2)
		{
			Judge();
		}
		else if(RxData2 == 0xc1)
		{
			Serial_USART1_SendByte('1');//给3861发消息开始加热
		}
		else if(RxData2 == 0xc2)
		{
			Serial_USART1_SendByte('0');//给3861发消息开始加热
		}
		else
		{
			Open_Box();
		}
		RxData2 = 0xff;
	}
}
void TIM3_IRQHandler(void)//定时器3中断函数（计时）
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		sec++;
		if(sec >= 60)
		{
			sec = 0;
			min++;
			if(min >= 60)
			{
				min = 0;
				hour++;
				if(hour >= 24)
				{
					hour = 0;
				}
			}

		}	
		printf("start.time.txt=\"%.2d:%.2d:%.2d\"\xff\xff\xff", hour, min, sec);
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);//标志位清零
	}
}
