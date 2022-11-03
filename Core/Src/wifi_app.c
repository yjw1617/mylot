#include "common_dev.h"
#include "common_event.h"
#include "wifi_app.h"
#include "message.h"
#include <stdio.h>
typedef struct Wifi_app_t{
	QueueHandle_t queue;
}Wifi_app_t;

typedef struct Wifi_t{
	int8_t fd;
	uint8_t status;
}Wifi_t;
static Wifi_app_t wifi_app;
static Wifi_t wifi;
//驱动函数来回调结果通过消息队列来通知到应用层
void event_handler(void *event_handler_arg, uint16_t event_type, int32_t event_id, void* event_data, uint16_t event_data_len){
	if(event_type == Event_Type_Wifi && event_id == Event_Id_Connect_Suc) {
		Error_Check(errQUEUE_FULL, xQueueSend( wifi_app.queue, "wifi connect net suc", portMAX_DELAY));
  }
	
}
static int8_t write_fd_event(int8_t fd, uint16_t event_type, uint16_t event_id, uint8_t* buf, uint16_t buf_len){
	Common_Event event1 = {
			.type = Event_Type_Wifi,
			.id = Event_Id_Connect,
			.data = buf,
			.data_len = buf_len,
	};
	Error_Check(-1, common_write(fd, &event1, sizeof(event1)));
	return 0;
}
int wifi_app_main(){
		wifi_app.queue = xQueueCreate(3, 25);
//		xQueueSend( QueueHandle_t xQueue, 
// const void * pvItemToQueue, 
// TickType_t xTicksToWait );
	
		//注册事件
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Connect, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Connect_Suc, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Wakeup, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Reset, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Reset_Suc, event_handler, NULL));
		wifi.fd = common_open((uint8_t*)"LeiNuoWifi1");
		LOG("common_open fd = %d\r\n", wifi.fd);
		Error_Check(-1, wifi.fd);
	  
		write_fd_event(wifi.fd, Event_Type_Wifi, Event_Id_Connect, NULL, 0);
		
		uint8_t buf[25] = {0};
		BaseType_t ret = 0;
		for(;;){
			ret = xQueueReceive(wifi_app.queue, buf, portMAX_DELAY);
			if(ret == pdPASS){
				if(strncmp(buf, "wifi connect net suc", strlen("wifi connect net suc")) == 0){
					LOG("app wifi connect suc\r\n");
				}
			}
		}
}