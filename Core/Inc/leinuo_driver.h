#pragma once
#include "message.h"
#include "common_dev.h"
#include "common_timer.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"

typedef struct LeiNuoWifi_dev{
	Dev dev;
	uint8_t status;//wifi状态
	
	SemaphoreHandle_t mutex;
	
	Common_Timer timer;
}LeiNuoWifi_dev;

int8_t leinuo_dev_init();

operations* leinuo_get_mydev_p();

uint8_t leinuo_is_leinuo_protocol(uint8_t* frame);
