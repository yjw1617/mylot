#include "gui_app.h"
#include "common_dev.h"
#include "common_event.h"
#include "message.h"
#include <string.h>
#include "wifi_app.h"
typedef struct Gui_app_t{
	QueueHandle_t queue;
}Gui_app_t;

typedef struct Gui_t{
	int8_t fd;
	uint8_t status;
}Gui_t;
static Gui_app_t gui_app;
static Gui_t gui;

//驱动函数来回调结果通过消息队列来通知到应用层
static void event_handler(void *event_handler_arg, uint16_t event_type, int32_t event_id, void* event_data, uint16_t event_data_len){
	if(event_type == Event_Type_Gui && event_id == Event_Id_Gui_Touch_Wifi_Connect) {
		Error_Check(errQUEUE_FULL, xQueueSend( gui_app.queue, "event_gui_touch_wifi_connect", portMAX_DELAY));
  }
}
static int8_t write_fd_event(int8_t fd, uint16_t event_type, uint16_t event_id, uint8_t* buf, uint16_t buf_len){
	Common_Event event1 = {
			.type = Event_Type_Wifi,
			.id = Event_Id_Wifi_Connect,
			.data = buf,
			.data_len = buf_len,
	};
	Error_Check(-1, common_write(fd, &event1, sizeof(event1)));
	return 0;
}
int gui_app_main(){
	gui_app.queue = xQueueCreate(3, 40);
	//注册事件
	Error_Check(-1, common_event_handler_register(Event_Type_Gui, Event_Id_Gui_Touch_Wifi_Connect, event_handler, NULL));
//	gui.fd = common_open((uint8_t*)"LeiNuoWifi1");
	LOG("common_open fd = %d\r\n", gui.fd);
	Error_Check(-1, gui.fd);
	
	uint8_t buf[25] = {0};
	BaseType_t ret = 0;
	
	//wifiapp的消息队列
	QueueHandle_t wifi_app_queue = app_get_wifiApp_queue();
	for(;;){
		ret = xQueueReceive(gui_app.queue, buf, 10);
			if(ret == pdPASS){
				if(strncmp(buf, "event_gui_touch_wifi_connect", strlen("event_gui_touch_wifi_connect")) == 0){
					//发送消息给wifi任务让其连接wifi
					xQueueSend(wifi_app_queue, "event_wifi_connect", portMAX_DELAY);
				}
				if(strncmp(buf, "event_gui_touch_net_connect", strlen("event_gui_touch_net_connect")) == 0){
					
				}
			}
	}
}