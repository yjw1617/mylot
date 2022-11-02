#pragma once
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

typedef struct Common_Task{
	xTaskHandle handle;
	uint32_t uxHighWaterMark;
	uint32_t minimumEverFreeHeapSize;
	uint32_t (*common_get_high_water_mark)(struct Common_Task* p_task);
	uint32_t (*common_get_free_heap_size)(struct Common_Task* p_task);
}Common_Task;
void common_task_init(Common_Task* p_task, const void* const handle);
