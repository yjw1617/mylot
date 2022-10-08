#pragma once
#include "message.h"
#include "common_dev.h"
#include "common_timer.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"





#define WIFI_NAME_XIAOYI "leinuowifi"
#define XIAOYI_TIMER_NAME_LEN 20

#define XIAOYI_MESSAGE_HEAD1 0xaa
#define XIAOYI_MESSAGE_HEAD2 0x33

typedef struct XiaoyiWifi_dev{
	Dev dev;
	SemaphoreHandle_t mutex;
	Common_Timer timer;
}XiaoyiWifi_dev;

void leinuo_init(XiaoyiWifi_dev* const mydev, uint16_t dev_addr, const uint8_t* const dev_name, const uint8_t* const timer_name);
operations* leinuo_get_mydev_p();

uint8_t leinuo_is_leinuo_protocol(uint8_t* frame);
