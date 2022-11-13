#include "dev_urgent_handle.h"
#include <stdint.h>
#include <string.h>
#include "message.h"
#include "usart.h"
#include "semphr.h"
#include <stdio.h>
#include "mcu_task.h"
#include "wifi_task.h"
#include "gui_task.h"
#include "common_task.h"

typedef struct Message_task{
	Common_Task common_task;
	QueueHandle_t Message_Queue;
}Message_task;

static Message_task g_message_task;

QueueHandle_t dev_urgent_handle_get_task_queue(){
	return g_message_task.Message_Queue;
}

static void dev_urgent_handle_init(){
	g_message_task.Message_Queue = xQueueCreate(MES_QUEUE_LEN, sizeof(Frame_t));
	if(g_message_task.Message_Queue == NULL){
		A_Log("dev_urgent_handle_queue create error\r\n");
	}
}

//void dev_urgent_handle(){
//	dev_urgent_handle_init();
//	uint8_t ret = 0;
//	Frame_t frame_temp = {};
//	int16_t addr_dest = 0;
//	Dev* dev_dest = NULL;
//	for(;;){
//		ret = xQueueReceive(g_message_task.Message_Queue, &frame_temp ,portMAX_DELAY);
//		if(ret == pdPASS){
//			//将消息的设备地址找出来
//			addr_dest = message_protocol_find_addr(&frame_temp);
//			//如果addr_dest！=-1直接处理
//			if(addr_dest != -1){
//				dev_dest = common_dev_find_dev_by_id(addr_dest);
//				if(dev_dest != NULL){
//					if(dev_dest->ops->msg_parse != NULL){
//						dev_dest->ops->msg_parse(dev_dest->mydev, &frame_temp.r_buf[frame_temp.index_useful], frame_temp.len);	
//					}
//				}
//			}else{
//				A_Log("addr_dest != -1\r\n");
//			}
//		}else{
//			A_Log("dev_urgent_handle_recv_mes_error\r\n");
//		}
//		memset(&frame_temp, 0, sizeof(Frame_t)); 
//	}
//}
