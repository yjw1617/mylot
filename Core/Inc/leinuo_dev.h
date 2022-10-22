#pragma once
#include "message.h"
#include "common_dev.h"
#include "common_timer.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"

typedef struct LeiNuoWifi_dev{
	uint8_t status;//wifi状态
	Dev dev;
	SemaphoreHandle_t mutex;
	Common_Timer timer;
}LeiNuoWifi_dev;

enum Leinuo_Cmd{
	Leinuo_Cmd_Wake = 0x00,
	Leinuo_Cmd_Sleep = 0x01,
	Leinuo_Cmd_Connect_Net = 0x02,
	Leinuo_Cmd_On = 0x03,
	Leinuo_Cmd_Off = 0x04,
	Leinuo_Cmd_Reset = 0x05,
};
int8_t leinuo_dev_init();

operations* leinuo_get_mydev_p();

uint8_t leinuo_is_leinuo_protocol(uint8_t* frame);