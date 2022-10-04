#pragma once
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
//任务句柄
typedef struct Common_Task{
	/*基本属性*/
	xTaskHandle handle;//任务句柄
	uint32_t uxHighWaterMark;//任务中最高水平还差多少溢出
	uint32_t minimumEverFreeHeapSize;//任务中最小空闲堆空间
	/*操作函数指针*/
	uint32_t (*common_get_high_water_mark)(struct Common_Task* p_task);
	uint32_t (*common_get_free_heap_size)(struct Common_Task* p_task);
}Common_Task;
void common_task_init(Common_Task* p_task, const void* const handle);
