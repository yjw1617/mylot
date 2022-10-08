#include "gui_poll_task.h"
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
	gui_poll_task_init(handle);
	mygui_poll_task_handle();
}