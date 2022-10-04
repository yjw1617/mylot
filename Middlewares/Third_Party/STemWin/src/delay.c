#include "delay.h"
#include "sys.h"
#include "tim.h"
#define DELAT_TIM TIM6
#define TIME_NAME htim6
//��ʼ���ӳٺ���
//��ʹ��OS��ʱ��,�˺������ʼ��OS��ʱ�ӽ���
//SYSTICK��ʱ�ӹ̶�ΪAHBʱ�ӵ�1/8
//SYSCLK:ϵͳʱ��Ƶ��
void delay_init()
{
	MX_TIM6_Init();
}								    

void delay_us(uint32_t nus)
{		
	MX_TIM6_Init();
	DELAT_TIM->CNT = nus -1;  //���ؼĴ�����ֵ
	DELAT_TIM->CR1 |= TIM_CR1_CEN; //
	HAL_TIM_Base_Start(&TIME_NAME);
	while(!(DELAT_TIM->SR & TIM_FLAG_UPDATE)); // �ж��Ƿ񴥷�����
	DELAT_TIM->SR = (uint16_t)~TIM_FLAG_UPDATE; //��ձ�־λ
	DELAT_TIM->CR1 &= TIM_CR1_CEN; // ��ԭ
	HAL_TIM_Base_Stop(&TIME_NAME);
}  
//��ʱnms
//nms:Ҫ��ʱ��ms��
//nms:0~65535
void delay_ms(uint16_t nms)
{
	//������ϵͳ��ʱ
#if(INCLUDE_vTaskDelayUntil == 1)
	TickType_t lasttick = xTaskGetTickCount();
	vTaskDelayUntil(&lasttick,nms);
#else
	//�����ʱ
	HAL_Delay(nms);
#endif
}

