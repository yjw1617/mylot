#include "wifi_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "mytestprotocol.h"
#include "xiaoyi.h"
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
	vTaskDelay(10);
	
	TestWifi_dev* testwifi_dev = dev_mydev_create(sizeof(TestWifi_dev));
	mytestwifi_init(testwifi_dev, MESSAGE_ADDR_WIFI_TEST, (uint8_t*)"test_wifi", (uint8_t*)"test_wifi_timer");
	dev_register(&(testwifi_dev->dev));
	
	XiaoyiWifi_dev* xiaoyi_dev= dev_mydev_create(sizeof(XiaoyiWifi_dev));
	xiaoyi_init(xiaoyi_dev, MESSAGE_ADDR_WIFI_XIAOYI, (uint8_t*)"xiaoyi_wifi", (uint8_t*)"xiaoyi_wifi2_timer");
	dev_register(&(xiaoyi_dev->dev));
	
	Message_t message_tmp = {0};
	int8_t ret = 0;
	
	for(;;){
		ret = xQueueReceive(g_wifi_task.Wifi_Queue, &message_tmp, portMAX_DELAY);
		if(ret == pdPASS){
			message_info(&message_tmp);
			Dev* dev_dest = dev_find_dev_by_addr(message_tmp.addr_dest);
			if(dev_dest != NULL){
				switch(message_tmp.cmd){
					case CMD_WIFI_ON:
						dev_dest->ops->on(dev_dest->mydev);
						break;
					case CMD_WIFI_CONNECT:
						dev_dest->ops->connect(dev_dest->mydev);
						break;
					case CMD_WIFI_RESET:
//						((XiaoyiWifi_dev*)(dev_dest->mydev))->timer.start(&((XiaoyiWifi_dev*)(dev_dest->mydev))->timer);
						dev_dest->ops->reset(dev_dest->mydev);
						break;
					case CMD_WIFI_OFF:
						dev_dest->ops->off(dev_dest->mydev);
						break;
					default:
						LOG("\r\n wifi dont know this cmd\r\n");
						break;
				}
			}else{
				LOG("dev_find_dev_by_addr failed\r\n");
			}
		}
	}
}
	