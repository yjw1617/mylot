#pragma once
#include "message_handle.h"
#include "common_dev.h"
#define WIFI_NAME_TEST "mytestwifi"

typedef struct TestWifi_dev{
	/* Ù–‘*/
	Dev dev;
	uint8_t name[10];
}TestWifi_dev;
void mytestwifi_init(TestWifi_dev* const mydev, uint16_t dev_addr, const uint8_t* const dev_name, const uint8_t* const timer_name);