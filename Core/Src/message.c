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
	//使能串口1，2，3空闲中断
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	//	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
	//	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
	//开启DMA搬运
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
	//判断是否产生空闲中断
	if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) == SET){
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);//清除中断标志
		//停止DMA
		HAL_UART_DMAStop(&huart1);
		//将接受长度求出
		uart1_rece_len = FRAME_MAX_LEN - hdma_usart1_rx.Instance->NDTR;//试试这里能不能通过strlen求出长度
		
		//将接受的数据添加到uart1_msg_S中,每次对USArt1进行读取后都需要清空缓冲区
		frame1_tmp.len = uart1_rece_len;
		
		//寻找uart1_Frame_buf中有效部分
		memcpy((&frame1_tmp.r_buf[0]), uart1_Frame_buf, uart1_rece_len);
		printf("len = %d\r\n", frame1_tmp.len);
		//检验消息帧
		for(uint8_t i = 0; i < frame1_tmp.len; i++){
			if(frame1_tmp.r_buf[i] == 0xaa && frame1_tmp.r_buf[i + 1] == 0x33){
				Message_make_fromISR(&message_tmp, &(frame1_tmp.r_buf[i]));
				//校验这包消息
				uint8_t ret = Message_check(&message_tmp);//长度位payload长度+8
				if(ret){//如果检验成功
					//将消息打入到消息队列中
					if(xQueueSendFromISR(Message_Queue, &message_tmp, NULL) != pdTRUE){
						printf("xQueueSendFromISR error\r\n");
					}
				}
			}
		}
		//将DMA_Usart1_RxBuffer清空
		memset(uart1_Frame_buf, 0, FRAME_MAX_LEN);
		memset(&message_tmp, 0, sizeof(Message_t));
		memset(&frame1_tmp, 0, sizeof(Frame_t));
		//再次开启DMA
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
		if(ret == pdPASS){//处理uart1接收到的消息
			switch(message_tmp.addr_dest){
				case ADDR_MCU:
					//将消息给到mcu消息队列里面
					mcu_mes_deal(&message_tmp);
					break;
				case ADDR_WIFI:
					
					//将消息给到wifi消息队列里面
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
