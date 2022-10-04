#include "message.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "usart.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include "mcu_task.h"
#include "wifi_task.h"
#include "gui_task.h"
#include "common_task.h"

typedef struct Message_task{
	Common_Task common_task;
	QueueHandle_t Message_Queue;
	uint8_t uart1_Frame_buf[FRAME_MAX_LEN];
}Message_task;

static Message_task g_message_task;

extern DMA_HandleTypeDef hdma_usart1_rx;

static char message_check(const Message_t* const mes);
static void message_make_fromISR(Message_t* const frame, const uint8_t* const buf);

void message_init(){
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	//	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
	//	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart1, g_message_task.uart1_Frame_buf, FRAME_MAX_LEN);
	//	HAL_UART_Receive_DMA(&huart2, uart2_mes_buf, R_MAX_LEN);
	//	HAL_UART_Receive_DMA(&huart3, uart3_mes_buf, R_MAX_LEN);
	g_message_task.Message_Queue = xQueueCreate(MES_QUEUE_LEN, MES_QUEUE_ITEM_SIZE);
	if(g_message_task.Message_Queue == NULL){
		LOG("\r\nxQueueCreate g_message_task.Message_Queue error\r\n");
	}else{
		LOG("\r\nxQueueCreate g_message_task.Message_Queue success\r\n");
	}
}

uint8_t message_queue(Message_t *mes, uint32_t delay_ms){
	return xQueueSend(g_message_task.Message_Queue, mes, delay_ms);
}

uint8_t message_dequeue(Message_t *mes, uint32_t delay_ms){
	return xQueueReceive( g_message_task.Message_Queue, mes, delay_ms);
}

void uart1_interrupt_handle(){
	Frame_t frame1_tmp;
	uint16_t uart1_rece_len = 0;
	Message_t message_tmp = {0};

	if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) == SET){
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);

		HAL_UART_DMAStop(&huart1);

		uart1_rece_len = FRAME_MAX_LEN - hdma_usart1_rx.Instance->NDTR;
		
		
		frame1_tmp.len = uart1_rece_len;
		

		memcpy((&frame1_tmp.r_buf[0]), g_message_task.uart1_Frame_buf, uart1_rece_len);
		for(uint8_t i = 0; i < frame1_tmp.len; i++){
			if(frame1_tmp.r_buf[i] == 0xaa && frame1_tmp.r_buf[i + 1] == 0x33){
				message_make_fromISR(&message_tmp, &(frame1_tmp.r_buf[i]));

				uint8_t ret = message_check(&message_tmp);
				if(ret){
				
					if(xQueueSendFromISR(g_message_task.Message_Queue, &message_tmp, NULL) != pdTRUE){
						LOG("\r\nxQueueSendFromISR error\r\n");
					}
				}
			}
		}

		memset(g_message_task.uart1_Frame_buf, 0, FRAME_MAX_LEN);
		memset(&message_tmp, 0, sizeof(Message_t));
		memset(&frame1_tmp, 0, sizeof(Frame_t));

		HAL_UART_Receive_DMA(&huart1, g_message_task.uart1_Frame_buf, FRAME_MAX_LEN);
	}
}
static char message_check(const Message_t* const mes){
	return 1;
}
static void message_make_fromISR(Message_t* const frame, const uint8_t* const buf){
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
void message_handle(const void* const handle){
	message_init();
	Message_t message_tmp = {0};
	int8_t ret = 0;
	uint8_t i = 0;
	for(;;){
		ret = message_dequeue(&message_tmp, portMAX_DELAY);
		if(ret == pdPASS){
			switch(message_tmp.type){
				case MES_TYPE_MCU:
					mcu_mes_deal(&message_tmp);
					break;
				case MES_TYPE_WIFI:
					ret = xQueueSend(wifi_get_Wifi_Queue(), &message_tmp, 0);
					if(ret != pdPASS){
						LOG("Wifi_queue full\r\n");
					}else{
						LOG("Wifi_insert success\r\n");
					}
					break;
				case MESSAGE_TYPE_GUI:
					ret = xQueueSend(gui_get_gui_Queue(), &message_tmp, 0);
					if(ret != pdPASS){
						LOG("Gui_queue full\r\n");
					}else{
						LOG("Gui_insert success\r\n");
					}
					break;
				default:
					LOG("Unknown message\r\n");
					break;
			}
		}
  }
}

void message_info(const Message_t* const mes){
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

void message_send(const Message_t* const mes){
	
}
