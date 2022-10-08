#include "message.h"

#include "usart.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include "mcu_task.h"
#include "wifi_task.h"
#include "gui_task.h"
#include "common_task.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
extern DMA_HandleTypeDef hdma_usart1_rx;

typedef struct Message_task{
	Common_Task common_task;
	QueueHandle_t Message_Queue;
	uint8_t uart1_Frame_buf[FRAME_MAX_LEN];
	Message_protocol_controller message_protocol_controller;
}Message_task;

static Message_task g_message_task;

//消息协议管理函数
static void* message_protocol_create(uint16_t size);

static uint8_t message_protocol_del(Message_protocol* protocol);

static int8_t message_protocol_register(Message_protocol* protocol);

static Message_protocol* message_find_protocol_by_name(uint8_t* protocol_name);

static uint8_t* message_protocol_find_type(Frame_t* frame);
//end

//消息相关接口
static char message_check(const Message_t* const mes);
static void message_make_fromISR(Message_t* const frame, const uint8_t* const buf);
//end

//消息协议管理函数
static void* message_protocol_create(uint16_t size){
	return pvPortMalloc(size);
}

static int8_t message_protocol_register(Message_protocol* protocol){
	for(uint16_t i = 0; i < Message_Protocol_Max_Num; i++){
		if(g_message_task.message_protocol_controller.protocols[i] == 0){
			g_message_task.message_protocol_controller.protocols[i] = protocol;
			g_message_task.message_protocol_controller.num++;
			LOG("protocol success\r\n");
			return pdTRUE;
		}
	}
	LOG("protocol_register buf full\r\n");
	return pdFALSE;
}

static uint8_t unregister_dev(Message_protocol* protocol){
	for(uint16_t i = 0; i < Message_Protocol_Max_Num; i++){
		if(protocol == g_message_task.message_protocol_controller.protocols[i]){
			g_message_task.message_protocol_controller.protocols[i] = 0;
		}
	}
}

static uint8_t message_protocol_del(Message_protocol* protocol){
	vPortFree(protocol);
}

static Message_protocol* message_find_protocol_by_name(uint8_t* protocol_name){
	for(uint16_t i = 0; i < Message_Protocol_Max_Num; i++){
		if(memcmp(protocol_name, g_message_task.message_protocol_controller.protocols[i]->name, strlen((char*)protocol_name))){
			return g_message_task.message_protocol_controller.protocols[i];
		}
	}
	LOG("protocol_find_dev_by_name\r\n");
	return NULL;
}

static uint8_t* message_protocol_find_type(Frame_t* frame){
	//Determines the length of the received message
//	assert(frame->len <= FRAME_MAX_LEN);
	for(uint8_t i = 0; i < frame->len; i++){
		for(uint8_t j = 0; j < Message_Protocol_Max_Num; j++){
			if(frame->r_buf[i] == g_message_task.message_protocol_controller.protocols[j]->head1){//message head1 ok
				//message head2 ok  
				if(frame->r_buf[i+1] == g_message_task.message_protocol_controller.protocols[j]->head2){
					return g_message_task.message_protocol_controller.protocols[j]->name;
				}
				//message end ok
				if(frame->r_buf[frame->r_buf[i + g_message_task.message_protocol_controller.protocols[j]->len_index] + g_message_task.message_protocol_controller.protocols[j]->len_index_more] == g_message_task.message_protocol_controller.protocols[j]->end){
					return g_message_task.message_protocol_controller.protocols[j]->name;
				}
			}
		}
	}
	return 0;
}
//end

void message_init(){
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	//	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
	//	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart1, g_message_task.uart1_Frame_buf, FRAME_MAX_LEN);
	//	HAL_UART_Receive_DMA(&huart2, uart2_mes_buf, R_MAX_LEN);
	//	HAL_UART_Receive_DMA(&huart3, uart3_mes_buf, R_MAX_LEN);
	g_message_task.Message_Queue = xQueueCreate(MES_QUEUE_LEN, sizeof(Frame_t));
	if(g_message_task.Message_Queue == NULL){
		LOG("\r\nxQueueCreate g_message_task.Message_Queue error\r\n");
	}else{
		LOG("\r\nxQueueCreate g_message_task.Message_Queue success\r\n");
	}
}

void uart1_interrupt_handle(){
	Frame_t frame1_tmp;
	if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) == SET){
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);
		HAL_UART_DMAStop(&huart1);
		frame1_tmp.len = FRAME_MAX_LEN - hdma_usart1_rx.Instance->NDTR;
		memcpy((&frame1_tmp.r_buf[0]), g_message_task.uart1_Frame_buf, frame1_tmp.len);
		//将接收到的消息添加到消息队列
		if(xQueueSendFromISR(g_message_task.Message_Queue, &frame1_tmp, NULL) != pdTRUE){
				LOG("\r\nxQueueSendFromISR error\r\n");
		}
		//清空g_message_task.uart1_Frame_buf
		memset(g_message_task.uart1_Frame_buf, 0, FRAME_MAX_LEN);
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
//	assert_param(4>6);
//	assert(4 > 6);
//	perror("hello\r\n");
//	uint8_t j = 6 / 0;
//	LOG("err = %s\r\n", strerror(errno)); 
	//创建雷诺消息结构
	Message_protocol protocol_leinuo = {
		.name = "leinuo1",
		.head1 = 0xaa,
		.head2 = 0x33,
		.len_index = 6,
		.end = 0x00,
	};
	Message_protocol* message_protocol_leinuo = message_protocol_create(sizeof(Message_protocol));
	memcpy(message_protocol_leinuo, &protocol_leinuo, sizeof(Message_protocol));
	message_protocol_register(message_protocol_leinuo);
	
	
	//创建yy消息结构
	Message_protocol protocol_yy = {
		.name = "yy1",
		.head1 = 0xaa,
		.head2 = 0x33,
		.len_index = 6,
		.len_index_more = 8,
		.end = 0x66,
	};
	Message_protocol* message_protocol_yy = message_protocol_create(sizeof(Message_protocol));
	memcpy(message_protocol_yy, &protocol_yy, sizeof(Message_protocol));
	message_protocol_register(message_protocol_yy);
	
	
	Frame_t frame_temp = {};
	int8_t ret = 0;
	uint8_t i = 0;
	for(;;){
		ret = xQueueReceive(g_message_task.Message_Queue, &frame_temp ,portMAX_DELAY);
		if(ret == pdPASS){
			//将消息给到协议库查询协议种类
			LOG("mes type = %s\r\n",message_protocol_find_type(&frame_temp));
//			switch(message_tmp.type){
//				case MESSAGE_TYPE_MCU:
//					mcu_mes_deal(&message_tmp);
//					break;
//				case MESSAGE_TYPE_WIFI:
//					ret = xQueueSend(wifi_get_Wifi_Queue(), &message_tmp, 0);
//					if(ret != pdPASS){
//						LOG("Wifi_queue full\r\n");
//					}else{
//						LOG("Wifi_insert success\r\n");
//					}
//					break;
//				case MESSAGE_TYPE_GUI:
//					ret = xQueueSend(gui_get_gui_Queue(), &message_tmp, 0);
//					if(ret != pdPASS){
//						LOG("Gui_queue full\r\n");
//					}else{
//						LOG("Gui_insert success\r\n");
//					}
//					break;
//				default:
//					LOG("Unknown message\r\n");
//					break;
//			}
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
