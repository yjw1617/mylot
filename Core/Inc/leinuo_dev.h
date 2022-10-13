#pragma once
#include "message.h"
#include "common_dev.h"
#include "common_timer.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"

typedef struct Leinuo_Msg{
	uint8_t head1;
	uint8_t head2;
	uint8_t addr_src;
	uint8_t addr_dest;
	uint8_t type;
	uint8_t cmd;
	uint16_t len;
	uint8_t payload[PAYLOAD_MAX_LEN];
	uint8_t check_num;
}Leinuo_Msg;

typedef struct LeiNuoWifi_dev{
	uint8_t status;//wifi状态
	Dev dev;
	QueueHandle_t Message_Queue;
	SemaphoreHandle_t mutex;
	Common_Timer timer;
}LeiNuoWifi_dev;

int8_t leinuo_dev_init();

operations* leinuo_get_mydev_p();

uint8_t leinuo_is_leinuo_protocol(uint8_t* frame);