#include "message_handle.h"
#include "message.h"
#include "usart.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include "mcu_task.h"
#include "wifi_task.h"
#include "gui_task.h"
#include "common_task.h"
#include "dev_urgent_handle.h"
extern DMA_HandleTypeDef hdma_usart1_rx;

typedef struct Message_task{
	Common_Task common_task;
	QueueHandle_t Message_Queue;
	uint8_t uart1_Frame_buf[FRAME_MAX_LEN];
}Message_task;

static Message_task g_message_task;

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
static void message_init(){
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

void message_handle(const void* const handle){
	message_init();
//	LOG("err = %s\r\n", strerror(errno)); 
	//创建并添加雷诺消息模型
	Message_protocol* message_protocol_leinuo = message_protocol_create(sizeof(Message_protocol));
	//下面为一个协议支持两个类型的设备
	message_protocol_copy(message_protocol_leinuo, 
	(Message_protocol){
		.name = "leinuo", 
		.type = Protocol_Type_G_Gui , 
		.head1=0xaa, 
		.head2=0x33, 
		.len_index = 6, 
		.len_index_more = 8, 
		.end = 0x66
	});
	message_protocol_register(message_protocol_leinuo);
	
	//创建并添加yy消息模型
	Message_protocol* message_protocol_yy = message_protocol_create(sizeof(Message_protocol));
	message_protocol_copy(message_protocol_yy, 
	(Message_protocol){
		.name = "yy", 
		.type = Protocol_Type_Wifi, 
		.head1=0xaa, 
		.head2=0x55, 
		.len_index = 6, 
		.len_index_more = 8, 
		.end = 0x55
	});
	message_protocol_register(message_protocol_yy);
	
	//这是自己mcu内部用的协议
	Message_protocol* message_protocol_mcu = message_protocol_create(sizeof(Message_protocol));
	message_protocol_copy(message_protocol_mcu, 
	(Message_protocol){
		.name = "mcu", 
		.type = MESSAGE_TYPE_MCU, 
		.head1=0xff, 
		.head2=0x55, 
		.len_index = 6, 
		.len_index_more = 8, 
		.dest_addr_index = 3,
	});
	message_protocol_register(message_protocol_mcu);
	
	//这是mygui用的协议
	Message_protocol* message_protocol_mygui = message_protocol_create(sizeof(Message_protocol));
	message_protocol_copy(message_protocol_mygui, 
	(Message_protocol){
		.name = "mygui", 
		.type = MESSAGE_TYPE_MCU, 
		.head1=0xbb, 
		.head2=0x44, 
		.len_index = 6, 
		.len_index_more = 8, 
		.dest_addr_index = 3,
	});
	message_protocol_register(message_protocol_mygui);
	
	
	//这是mygui用的协议
	Message_protocol* message_protocol_kkk = message_protocol_create(sizeof(Message_protocol));
	message_protocol_copy(message_protocol_kkk, 
	(Message_protocol){
		.name = "kkk", 
		.type = MESSAGE_TYPE_MCU, 
		.head1=0x33, 
		.head2=0x66, 
		.len_index = 4, 
		.len_index_more = 6, 
		.dest_addr_index = 0,
	});
	message_protocol_register(message_protocol_kkk);
	
	Frame_t frame_temp = {};
	int8_t ret = 0;
	uint8_t i = 0;
	int16_t dest_addr = 0;
	uint8_t message_type = 0;
	Dev* dev = 0;
	uint8_t index_useful = 0;
	uint8_t temp_data[FRAME_MAX_LEN] = {};
	dev_controller* dev_con = NULL;
	QueueHandle_t urgent_handle_get_task_queue = NULL;
	uint8_t mes_type = 0;
	for(;;){
		ret = xQueueReceive(g_message_task.Message_Queue, &frame_temp ,portMAX_DELAY);
		
		if(ret == pdPASS){
			//解析一下消息的类型是属于紧急消息还是普通消息，如果紧急加入紧急任务中处理
			mes_type = message_protocol_find_type(&frame_temp);
			
			if(mes_type == Protocol_Type_Urgency){
				urgent_handle_get_task_queue = dev_urgent_handle_get_task_queue();
				if(urgent_handle_get_task_queue!= NULL){
					//将消息插入紧急任务中处理
					ret = xQueueSend(urgent_handle_get_task_queue, &frame_temp, 0);
					if(ret != pdPASS){
						LOG("xQueueSend(urgent_handle_get_task_queue, &frame_temp, 0) error\r\n");
					}
				}else{
					LOG("urgent_handle_get_task_queue\r\n");
				}
			}else{
				//解析出各个消息的目标地址,根据目标地址发送到设备的消息队列中,如果返回值为-1,那么代表着这个类型的消息不支持addr_dest
				dest_addr = message_protocol_find_addr(&frame_temp);
				//如果不紧急就加入各个设备的queue中等待处理
				if(dest_addr == -1){//由于消息并没有addr_dest，所以将此消息广播给所有设备
					dev_con = common_dev_get_controller();
					for(uint8_t i = 0; i < DEV_MAX_NUM; i++){
						if(dev_con->dev[i] != NULL){
							if(dev_con->dev[i]->Message_Queue != NULL){
								ret = xQueueSend(dev_con->dev[i]->Message_Queue, &frame_temp, 0);
								if(ret != pdPASS){
									LOG("dev->Message_Queue full\r\n");
								}
							}
						}
					}
				}else{
					//将消息发送给指定设备的消息队列中等待处理
					dev = common_dev_find_dev_by_addr(dest_addr);
					if(dev != NULL){
						if(dev->Message_Queue != NULL){
							ret = xQueueSend(dev->Message_Queue, &frame_temp, 0);
							if(ret != pdPASS){
								LOG("dev->Message_Queue full\r\n");
							}
						}
					}else{
						LOG("common_dev_find_dev_by_addr error\r\n");
					}
				}
			}
			memset(&frame_temp, 0, sizeof(Frame_t)); 
		}
  }
}

