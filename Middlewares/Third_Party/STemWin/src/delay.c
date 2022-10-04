#include "delay.h"
#include "sys.h"
#include "tim.h"
#define DELAT_TIM TIM6
#define TIME_NAME htim6
//初始化延迟函数
//当使用OS的时候,此函数会初始化OS的时钟节拍
//SYSTICK的时钟固定为AHB时钟的1/8
//SYSCLK:系统时钟频率
void delay_init()
{
	MX_TIM6_Init();
}								    

void delay_us(uint32_t nus)
{		
	MX_TIM6_Init();
	DELAT_TIM->CNT = nus -1;  //重载寄存器赋值
	DELAT_TIM->CR1 |= TIM_CR1_CEN; //
	HAL_TIM_Base_Start(&TIME_NAME);
	while(!(DELAT_TIM->SR & TIM_FLAG_UPDATE)); // 判断是否触发重载
	DELAT_TIM->SR = (uint16_t)~TIM_FLAG_UPDATE; //清空标志位
	DELAT_TIM->CR1 &= TIM_CR1_CEN; // 还原
	HAL_TIM_Base_Stop(&TIME_NAME);
}  
//延时nms
//nms:要延时的ms数
//nms:0~65535
void delay_ms(uint16_t nms)
{
	//带操作系统延时
#if(INCLUDE_vTaskDelayUntil == 1)
	TickType_t lasttick = xTaskGetTickCount();
	vTaskDelayUntil(&lasttick,nms);
#else
	//裸机延时
	HAL_Delay(nms);
#endif
}

