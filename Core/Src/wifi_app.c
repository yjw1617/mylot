#include "common_dev.h"
#include "common_event.h"
#include "wifi_app.h"
#include "message.h"
#include <stdio.h>

//驱动函数来回调
void event_handler(void *event_handler_arg, uint16_t event_type, int32_t event_id, void* event_data, uint16_t event_data_size){
	if (event_type == Event_Type_Wifi && event_id == Event_Id_Connect_Suc) {
        LOG("connect net success\r\n");
				LOG("data = %s\r\n", event_data);
  }
}
int app_main(){
		int8_t ret = -1;
		//注册事件
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Connect, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Connect_Suc, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Wakeup, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Reset, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Wifi, Event_Id_Reset_Suc, event_handler, NULL));
		int8_t fd = common_open((uint8_t*)"LeiNuoWifi1");
		LOG("common_open fd = %d\r\n", fd);
		if(fd == -1){
			LOG("common_open error\r\n");
			return -1;
		}
		Common_Event event1 = {
			.type = Event_Type_Wifi,
			.id = Event_Id_Connect,
		};
		ret = common_write(fd, &event1, sizeof(event1));
		if(ret == -1){
			LOG("common_write error\r\n");
			return -1;
		}
}