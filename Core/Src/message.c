#include "message.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "usart.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include "mcu.h"
#include "wifi.h"

extern DMA_HandleTypeDef hdma_usart1_rx;

static uint8_t uart1_Frame_buf[FRAME_MAX_LEN];

static char Message_check(const Message_t* const mes);
static void Message_make_fromISR(Message_t* const frame, const uint8_t* const buf);

static QueueHandle_t Message_Queue;
static void Message_init(){
	//ʹ�ܴ���1��2��3�����ж�
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	//	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
	//	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
	//����DMA����
	HAL_UART_Receive_DMA(&huart1, uart1_Frame_buf, FRAME_MAX_LEN);
	//	HAL_UART_Receive_DMA(&huart2, uart2_mes_buf, R_MAX_LEN);
	//	HAL_UART_Receive_DMA(&huart3, uart3_mes_buf, R_MAX_LEN);
	Message_Queue = xQueueCreate(MES_QUEUE_LEN, MES_QUEUE_ITEM_SIZE);
}

uint8_t Message_queue(Message_t *mes, uint32_t delay_ms){
	return xQueueSend(Message_Queue, &mes, pdMS_TO_TICKS(delay_ms));
}

uint8_t Message_dequeue(Message_t *mes, uint32_t delay_ms){
	return xQueueReceive( Message_Queue, &mes, pdMS_TO_TICKS(delay_ms));
}

void uart1_interrupt_handle(){
	Frame_t frame1_tmp;
	uint16_t uart1_rece_len = 0;
	Message_t message_tmp = {0};
	//�ж��Ƿ���������ж�
	if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) == SET){
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);//����жϱ�־
		//ֹͣDMA
		HAL_UART_DMAStop(&huart1);
		//�����ܳ������
		uart1_rece_len = FRAME_MAX_LEN - hdma_usart1_rx.Instance->NDTR;//���������ܲ���ͨ��strlen�������
		
		//�����ܵ�������ӵ�uart1_msg_S��,ÿ�ζ�USArt1���ж�ȡ����Ҫ��ջ�����
		frame1_tmp.len = uart1_rece_len;
		
		//Ѱ��uart1_Frame_buf����Ч����
		memcpy((&frame1_tmp.r_buf[0]), uart1_Frame_buf, uart1_rece_len);
		printf("len = %d\r\n", frame1_tmp.len);
		//������Ϣ֡
		for(uint8_t i = 0; i < frame1_tmp.len; i++){
			if(frame1_tmp.r_buf[i] == 0xaa && frame1_tmp.r_buf[i + 1] == 0x33){
				Message_make_fromISR(&message_tmp, &(frame1_tmp.r_buf[i]));
				//У�������Ϣ
				uint8_t ret = Message_check(&message_tmp);//����λpayload����+8
				if(ret){//�������ɹ�
					//����Ϣ���뵽��Ϣ������
					if(xQueueSendFromISR(Message_Queue, &message_tmp, NULL) != pdTRUE){
						printf("xQueueSendFromISR error\r\n");
					}
				}
			}
		}
		//��DMA_Usart1_RxBuffer���
		memset(uart1_Frame_buf, 0, FRAME_MAX_LEN);
		memset(&message_tmp, 0, sizeof(Message_t));
		memset(&frame1_tmp, 0, sizeof(Frame_t));
		//�ٴο���DMA
		HAL_UART_Receive_DMA(&huart1, uart1_Frame_buf, FRAME_MAX_LEN);
	}
}
static char Message_check(const Message_t* const mes){
	return 1;
}
static void Message_make_fromISR(Message_t* const frame, const uint8_t* const buf){
	frame->head1 = buf[0];
	frame->head2 = buf[1];
	frame->addr_src = buf[2];
	frame->addr_dest = buf[3];
	frame->type = buf[4];
	frame->cmd = buf[5];
	frame->len = buf[6];
	memcpy(frame->payload, &buf[7], frame->len);
	frame->check_num = buf[frame->len + 7];
}
void MessageHandle(void const * argument){
	Message_init();
	Message_t message_tmp = {0};
	int8_t ret = 0;
	uint8_t i = 0;
	for(;;){
		ret = Message_dequeue(&message_tmp, portMAX_DELAY);
		LOG("kkkkk\r\n");
		printf("ddd\r\n");
		if(ret == pdPASS){//����uart1���յ�����Ϣ
			switch(message_tmp.addr_dest){
				case ADDR_MCU:
					//����Ϣ����mcu��Ϣ��������
					mcu_mes_deal(&message_tmp);
					break;
				case ADDR_WIFI:
					
					//����Ϣ����wifi��Ϣ��������
//					ret = Wifi_queue(&message_tmp, 0);
//					(ret == pdPASS) ? LOG("WIFI_queue success") : LOG("WIFI_queue FULL\r\n");
					break;
				default:
					LOG("Unknown message\r\n");
					break;
			}
		}
  }
}

void log_frame(const Message_t* const mes){
	printf("\r\n");
	printf("fram.head1 = %.2x\r\n", mes->head1);
	printf("fram.head2 = %.2x\r\n", mes->head2);
	printf("fram.addr_src = %.2x\r\n", mes->addr_src);
	printf("frame->addr_dest = %.2x\r\n", mes->addr_dest);
	printf("frame->type = %.2x\r\n", mes->type);
	printf("frame->cmd = %.2x\r\n", mes->cmd);
	printf("frame->len = %d\r\n", mes->len);
	printf("payload: ");
	for(uint8_t i = 0; i < mes->len; i++){
		printf("%.2x ", mes->payload[i]);
	}
	printf("\r\nframe.check_num = %.2x\r\n", mes->check_num);
}

void Message_send(const Message_t* const mes){
	
}
