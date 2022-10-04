#pragma once
#include "message.h"
#include "common_dev.h"
#include "common_timer.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#define WIFI_NAME_XIAOYI "xiaoyiwifi"
#define XIAOYI_TIMER_NAME_LEN 20
typedef struct XiaoyiWifi_dev{
	Dev dev;
	SemaphoreHandle_t mutex;
	Common_Timer timer;
}XiaoyiWifi_dev;

void xiaoyi_init(XiaoyiWifi_dev* const mydev, uint16_t dev_addr, const uint8_t* const dev_name, const uint8_t* const timer_name);
operations* xiaoyi_get_mydev_p();
