#pragma once
#include "FreeRTOS.h"
#include "queue.h"
//#define Event_Gui_Touch_Wifi_Connect
int fingerprint_app_main();
QueueHandle_t app_get_fingerprintApp_queue();