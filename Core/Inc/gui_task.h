#pragma once
#include "message.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "common_dev.h"
#define GUI_QUEUE_LEN 2
#define GUI_QUEUE_ITEM_SIZE sizeof(Message_t)
#define GUI_MAX_NUM 10

void gui_handle(TaskHandle_t handle);
QueueHandle_t gui_get_gui_Queue();

