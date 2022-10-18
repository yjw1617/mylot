#pragma once
#include "message_handle.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "common_dev.h"
#define WIFI_QUEUE_LEN 2
#define WIFI_QUEUE_ITEM_SIZE sizeof(Frame_t)
#define WIFI_MAX_NUM 10

void wifi_handle(TaskHandle_t handle);
QueueHandle_t wifi_get_Wifi_Queue();
