#include "common_task.h"

static uint32_t common_get_high_water_mark(Common_Task* p_task){
	p_task->uxHighWaterMark = uxTaskGetStackHighWaterMark(p_task->handle);
	return p_task->uxHighWaterMark;
}

static uint32_t common_get_free_heap_size(Common_Task* p_task){
	p_task->minimumEverFreeHeapSize = xPortGetMinimumEverFreeHeapSize();
	return p_task->minimumEverFreeHeapSize;
}

void common_task_init(Common_Task* p_task, const void* const handle){
	p_task->handle = (xTaskHandle)handle;
	p_task->minimumEverFreeHeapSize = 0;
	p_task->uxHighWaterMark = 0;
	p_task->common_get_free_heap_size = common_get_free_heap_size;
	p_task->common_get_high_water_mark = common_get_high_water_mark;
}

