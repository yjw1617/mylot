#include "wifi_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "mytestprotocol_driver.h"
#include "leinuo_driver.h"
#include "common_task.h"
#include "leinuo_service.h"
typedef struct Wifi_task{
	Common_Task common_task;
	QueueHandle_t Wifi_Queue;
}Wifi_task;

static Wifi_task g_wifi_task;

static void wifi_init(TaskHandle_t handle){
	g_wifi_task.Wifi_Queue = xQueueCreate(WIFI_QUEUE_LEN , WIFI_QUEUE_ITEM_SIZE);
	common_task_init(&g_wifi_task.common_task, handle);
}

QueueHandle_t wifi_get_Wifi_Queue(){
	return  g_wifi_task.Wifi_Queue;
}




void wifi_handle(TaskHandle_t handle){
	wifi_init(handle);
	
	leinuo_driver_init();//初始化leinuo驱动层
	Frame_t frame_tmp = {0};
	int8_t ret = 0;
	uint8_t* name = 0;//消协议的名字
	uint8_t useful_index = 0;//一帧消息中第一个有效位的下标
	uint8_t buf[FRAME_MAX_LEN] = {};
	for(;;){
		ret = xQueueReceive(g_wifi_task.Wifi_Queue, &frame_tmp, portMAX_DELAY);
		if(ret == pdPASS){
			//解析消息
			name = message_protocol_find_name(frame_tmp.r_buf, frame_tmp.len, &useful_index);
			if(name != NULL){
				//将有效消息提取出来
				memcpy(buf, &frame_tmp.r_buf[useful_index], frame_tmp.len - useful_index);
				//根据名字来分拣消息给不同的设备
				if(!memcmp(name, "yy", strlen("yy"))){
					LOG("yy\r\n");
				}else if(!memcmp(name, "leinuo", strlen("leinuo"))){
					leinuo_deal_recv_msg(buf, frame_tmp.len - useful_index);
				}
			}
		}
	}
}
	