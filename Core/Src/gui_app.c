#include "gui_app.h"
#include "common_dev.h"
#include "common_event.h"
#include "message.h"
#include <string.h>
#include "wifi_app.h"
#include "fingerprint_app.h"
#include <stdio.h>
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
		common_write_queue_event(gui_app.queue, event_type, event_id, event_data, event_data_len);
}
static int8_t write_fd_event(int8_t fd, uint16_t event_type, uint16_t event_id, uint8_t* buf, uint16_t buf_len){
	Common_Event event = {
			.type = event_type,
			.id = event_id,
			.data = buf,
			.data_len = buf_len,
	};
	Error_Check(-1, common_write(fd, &event, sizeof(event)));
	return 0;
}


int gui_app_main(){
	gui_app.queue = xQueueCreate(3, sizeof(Common_Event));
	//注册事件
	Error_Check(-1, common_event_handler_register(Event_Type_Gui, Event_Id_Gui_Touch_Wifi_Connect, event_handler, NULL));
//	gui.fd = common_open((uint8_t*)"LeiNuoWifi1");
	A_Log("common_open fd = %d\r\n", gui.fd);
	Error_Check(-1, gui.fd);
	
	Common_Event app_event = {0};
	BaseType_t ret = 0;
	
	//wifiapp的消息队列
	QueueHandle_t wifi_app_queue = app_get_wifiApp_queue();
	//fp的消息队列
	QueueHandle_t fp_app_queue = app_get_fingerprintApp_queue();
	for(;;){
		ret = xQueueReceive(gui_app.queue, &app_event, sizeof(Common_Event));
			if(ret == pdPASS){
				switch(app_event.type){
					case Event_Type_Gui:
							switch(app_event.id){
								case Event_Id_Gui_Touch_Wifi_Connect:
//									common_write_queue_event(wifi_app_queue, 	Event_Type_Wifi, Event_Id_Wifi_Connect_Redo, NULL, 0);
									common_write_queue_event(fp_app_queue, 	Event_Type_Fingerprint, Event_Id_Fingerprint_Test, "start", strlen("start"));
									break;
							
							}
						break;						
				}
			}
			//销毁r_buf
			common_event_destroy(&app_event);
			memset(&app_event, 0, sizeof(Common_Event));
	}
}