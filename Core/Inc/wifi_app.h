#pragma once
#include "FreeRTOS.h"
#include "queue.h"
int wifi_app_main();
QueueHandle_t app_get_wifiApp_queue();