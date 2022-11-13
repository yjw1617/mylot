#pragma once
#include <stdint.h>
#include "message.h"
typedef struct Common_Payload{
	uint16_t timeout;
	uint16_t delay_ticks;
	uint8_t reload;
	uint8_t data[50];
}Common_Payload;
//#define Event_Type_Wifi   0x00
//#define Event_Type_Zigbee 0x01
//#define Event_Type_Gui    0x02
enum Event_Type{
	Event_Type_Wifi,
	Event_Type_Zigbee,
	Event_Type_Gui,
	Event_Type_Fingerprint,	
};
enum Event_Id{
	Event_Id_Wifi_Connect_Redo,
	Event_Id_Wifi_Connect,
	Event_Id_Wifi_Connect_Suc,
	Event_Id_Wifi_Connect_TimeOut,
	Event_Id_Wifi_Wakeup,
	Event_Id_Wifi_Reset,
	Event_Id_Wifi_Reset_Suc,
	Event_Id_Gui_Touch_Wifi_Connect,
	Event_Id_Fingerprint_Trigger,//指纹模块按下
	Event_Id_Fingerprint_Power,//指纹模块电源
	Event_Id_Fingerprint_Test,//检测指纹模块是否被按下
	Event_Id_Fingerprint_Test_Start,
	Event_Id_Fingerprint_AutoIdentify,//自动验证
};
//#define Event_Id_Wifi_Connect 			0x00
//#define Event_Id_Wifi_Connect_Suc 	0x01
//#define Event_Id_Wifi_Wakeup				0x02
//#define Event_Id_Wifi_Reset				0x03
//#define Event_Id_Wifi_Reset_Suc		0x04

typedef void (*event_handler_t)(void *event_handler_arg, uint16_t event_type, int32_t event_id, void* event_data, uint16_t event_data_len);
typedef struct Common_Event{
	int16_t type;
	int16_t id;
	event_handler_t handle;
	void* data;
	void* handle_args;
	uint16_t data_len;
	uint32_t ticks_to_wait;
}Common_Event;


int8_t common_event_handler_register(uint16_t event_type, uint32_t event_id, event_handler_t handle, void* event_handler_arg);

int8_t common_event_post(uint16_t event_type, uint32_t event_id, void* event_data, uint16_t event_data_lens, uint32_t ticks_to_wait, uint8_t reload_mode);

void common_event_init();

int8_t common_del_temp_event_by_eventType_and_eventId(uint16_t event_type, uint32_t event_id);

int8_t common_event_destroy(Common_Event* event);

int8_t common_write_queue_event(QueueHandle_t queue, uint16_t event_type, uint16_t event_id, uint8_t* buf, uint16_t buf_len);
