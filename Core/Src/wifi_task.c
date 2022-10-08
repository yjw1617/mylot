#include "wifi_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "mytestprotocol.h"
#include "leinuo.h"
#include "common_task.h"
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
	TestWifi_dev* testwifi_dev = dev_mydev_create(sizeof(TestWifi_dev));
	mytestwifi_init(testwifi_dev, MESSAGE_ADDR_WIFI_TEST, (uint8_t*)"test_wifi", (uint8_t*)"test_wifi_timer");
	dev_register(&(testwifi_dev->dev));
	
	XiaoyiWifi_dev* leinuo_dev= dev_mydev_create(sizeof(XiaoyiWifi_dev));
	leinuo_init(leinuo_dev, MESSAGE_ADDR_WIFI_XIAOYI, (uint8_t*)"leinuo_wifi", (uint8_t*)"leinuo_wifi_timer");
	dev_register(&(leinuo_dev->dev));
	
	XiaoyiWifi_dev* leinuo1_dev= dev_mydev_create(sizeof(XiaoyiWifi_dev));
	leinuo_init(leinuo1_dev, MESSAGE_ADDR_WIFI_XIAOYI1, (uint8_t*)"leinuo1_wifi1", (uint8_t*)"leinuo1_wifi1_timer");
	dev_register(&(leinuo1_dev->dev));
	
	Message_t message_tmp = {0};
	int8_t ret = 0;
	
	for(;;){
		ret = xQueueReceive(g_wifi_task.Wifi_Queue, &message_tmp, portMAX_DELAY);
		if(ret == pdPASS){
			message_info(&message_tmp);
			Dev* dev = dev_find_dev_by_addr(message_tmp.addr_dest);
			if(dev != NULL){
					dev->ops->ioctl(dev, message_tmp.cmd, (uint32_t)(message_tmp.payload), message_tmp.len);
			}else{
				LOG("dev_find_dev_by_addr failed\r\n");
			}
		}
	}
}
	