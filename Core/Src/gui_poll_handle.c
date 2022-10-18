#include "gui_poll_handle.h"
#include "mygui_api.h"
#include "common_task.h"
typedef struct Gui_poll_task{
	Common_Task common_task;
	QueueHandle_t Queue;
}Gui_poll_task;

static volatile Gui_poll_task g_gui_poll_task;

static void gui_poll_task_init(TaskHandle_t handle){
	g_gui_poll_task.Queue = xQueueCreate(GUI_QUEUE_LEN , GUI_QUEUE_ITEM_SIZE);
	common_task_init(&g_gui_poll_task.common_task, handle);
}

void gui_poll_handle(TaskHandle_t handle){
	WM_SetCreateFlags(WM_CF_MEMDEV);	
	GUI_Init();                     //初始化emWin
	GUI_UC_SetEncodeUTF8();         //使能utf-8编码
	mygui_MainCreate();
	while(1){
		GUI_TOUCH_Exec();	
		GUI_Delay(10);
		vTaskDelay(1);
	}
}