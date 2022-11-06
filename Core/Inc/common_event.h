#pragma once
#include <stdint.h>
#include "message.h"

#define Event_Type_Wifi   0x00
#define Event_Type_Zigbee 0x01
#define Event_Type_Gui    0x02

#define Event_Id_Wifi_Connect 			0x00
#define Event_Id_Wifi_Connect_Suc 	0x01
#define Event_Id_Wifi_Wakeup				0x02
#define Event_Id_Wifi_Reset				0x03
#define Event_Id_Wifi_Reset_Suc		0x04

#define Event_Id_Gui_Touch_Wifi_Connect 0x05
typedef void (*event_handler_t)(void *event_handler_arg, uint16_t event_type, int32_t event_id, void* event_data, uint16_t event_data_len);

typedef struct Common_Event{
	uint16_t type;
	uint16_t id;
//	uint8_t data[Common_Event_Data_Num];
	event_handler_t handle;
	void* data;
	void* handle_args;
	uint16_t data_len;
	uint32_t ticks_to_wait;
}Common_Event;


int8_t common_event_handler_register(uint16_t event_type, uint32_t event_id, event_handler_t handle, void* event_handler_arg);

int8_t common_event_post(uint16_t event_type, uint32_t event_id, void* event_data, uint16_t event_data_lens, uint32_t ticks_to_wait, uint8_t reload_mode);

void common_event_init();
