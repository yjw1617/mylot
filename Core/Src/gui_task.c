#include "gui_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "common_task.h"
#include "myGUI.h"
typedef struct Gui_task{
	Common_Task common_task;
	QueueHandle_t Queue;
}Gui_task;

static volatile Gui_task g_gui_task;

static void gui_task_init(TaskHandle_t handle){
	g_gui_task.Queue = xQueueCreate(GUI_QUEUE_LEN , GUI_QUEUE_ITEM_SIZE);
	common_task_init(&g_gui_task.common_task, handle);
}

QueueHandle_t gui_get_gui_Queue(){
	return  g_gui_task.Queue;
}

//wifi处理函数
void gui_handle(TaskHandle_t handle){
	gui_task_init(handle);
	//创建mygui设备
	MyGUI_dev* mygui_dev = dev_mydev_create(sizeof(MyGUI_dev));
	mygui_init(mygui_dev, MESSAGE_ADDR_MY_GUI, (uint8_t*)"my_gui", (uint8_t*)"my_gui_timer");
	dev_register(&(mygui_dev->dev));
	Message_t message_tmp = {0};//接收消息队列过来的消息
	int8_t ret = 0;
	for(;;){
		//接收消息
		ret = xQueueReceive(g_gui_task.Queue, &message_tmp, portMAX_DELAY);
		if(ret == pdPASS){//处理uart1接收到的消息
			//打印接收到的消息
			message_info(&message_tmp);
			Dev* dev = dev_find_dev_by_addr(message_tmp.addr_dest);
			if(dev != NULL){
				dev->ops->ioctl(dev->mydev, message_tmp.cmd, (uint32_t)message_tmp.payload, message_tmp.len);
			}else{
				LOG("dev_find_dev_by_addr failed\r\n");
			}
		}
	}
}