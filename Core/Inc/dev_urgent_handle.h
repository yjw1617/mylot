#pragma once
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
void dev_urgent_handle();
QueueHandle_t dev_urgent_handle_get_task_queue();