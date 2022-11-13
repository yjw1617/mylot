#include "common_dev.h"
#include "common_event.h"
#include "wifi_app.h"
#include "message.h"
#include <stdio.h>
#include <string.h>
typedef struct Wifi_app_t{
	QueueHandle_t queue;
	uint16_t temp_event_id;
}Wifi_app_t;

typedef struct Wifi_t{
	int8_t fd;
	uint8_t status;
}Wifi_t;


static Wifi_app_t wifi_app;
static Wifi_t wifi;


static int8_t write_fd_event(int8_t fd, uint16_t event_type, uint16_t event_id, uint8_t* buf, uint16_t buf_len);
//驱动函数来回调结果通过消息队列来通知到应用层
static void event_handler(void *event_handler_arg, uint16_t event_type, int32_t event_id, void* event_data, uint16_t event_data_len){
	switch(event_type){
		case Event_Type_Wifi:
				switch(event_id){
					case Event_Id_Wifi_Connect:
						common_write_queue_event(wifi_app.queue, Event_Type_Wifi, Event_Id_Wifi_Connect, event_data, event_data_len);
						break;
					case Event_Id_Wifi_Connect_TimeOut:
						common_write_queue_event(wifi_app.queue, Event_Type_Wifi, Event_Id_Wifi_Connect_TimeOut, event_data, event_data_len);//通知app
						write_fd_event(wifi.fd, Event_Type_Wifi, Event_Id_Wifi_Connect_TimeOut, "on", sizeof("on"));
						break;
					case Event_Id_Wifi_Connect_Suc:
						common_write_queue_event(wifi_app.queue, Event_Type_Wifi, Event_Id_Wifi_Connect_Suc, event_data, event_data_len);
						break;
				}
			break;			
	}
}
static int8_t write_fd_event(int8_t fd, uint16_t event_type, uint16_t event_id, uint8_t* buf, uint16_t buf_len){
	Common_Event event1 = {
			.type = event_type,
			.id = event_id,
			.data = buf,
			.data_len = buf_len,
	};
	A_Log("wifi write\r\n");
	Error_Check(-1, common_write(fd, &event1, sizeof(event1)));
	return 0;
}

QueueHandle_t app_get_wifiApp_queue(){
	return wifi_app.queue;
}

int wifi_app_main(){
		wifi_app.queue = xQueueCreate(3, sizeof(Common_Event));
		//注册事件
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Wifi_Connect, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Wifi_Connect_Suc, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Wifi_Wakeup, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Wifi_Reset, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Wifi_Reset_Suc, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Wifi_Connect_Redo, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Wifi_Connect_TimeOut, event_handler, NULL));
		wifi.fd = common_open((uint8_t*)"LeiNuoWifi1");
		Error_Check(-1, wifi.fd);
		uint8_t buf[25] = {0};
		BaseType_t ret = 0;
		
		Common_Event app_event = {0};
		Common_Payload payload = {0};
		for(;;){
			ret = xQueueReceive(wifi_app.queue, &app_event, sizeof(Common_Event));
			if(ret == pdPASS){
				switch(app_event.type){
					case Event_Type_Wifi:
							switch(app_event.id){
								case Event_Id_Wifi_Connect_Redo:
									payload.timeout = 15000;
								  payload.reload = 0;
									payload.delay_ticks = 0;
									write_fd_event(wifi.fd, Event_Type_Wifi, Event_Id_Wifi_Connect_Redo, (uint8_t*)&payload, sizeof(payload));
									A_Log("Event_Id_Wifi_Connect_Redo\r\n");
									break;
								case Event_Id_Wifi_Connect:
									A_Log("Event_Id_Wifi_Connect\r\n");
									break;
								case Event_Id_Wifi_Connect_TimeOut:
									A_Log("wifi connect timeout...\r\n");
									break;
								case Event_Id_Wifi_Connect_Suc:
									A_Log("wifi conect suc!!!\r\n");
									break;
							}
						break;			
				}
				//销毁r_buf
				common_event_destroy(&app_event);
				memset(&app_event, 0, sizeof(Common_Event));
				memset(&payload, 0, sizeof(Common_Payload));
			}
		}
}