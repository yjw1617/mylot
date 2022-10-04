#pragma once
#include <stdint.h>
#include "FreeRTOS.h"
#include "timers.h"
#define Common_Timer_Name_Len 20

typedef struct Common_Timer{

	uint8_t* name[Common_Timer_Name_Len];
	TimerHandle_t handle;
	TickType_t period;
	UBaseType_t autoReload;
	uint32_t id;
	TickType_t ticksToWait;
	TimerCallbackFunction_t callback_function;

	BaseType_t (*create)(struct Common_Timer* commom_timer);
	BaseType_t (*start)(struct Common_Timer* commom_timer);
	BaseType_t (*stop)(struct Common_Timer* commom_timer);
	BaseType_t (*Delete)(struct Common_Timer* commom_timer);
	BaseType_t (*change_period)(struct Common_Timer* commom_timer, TickType_t period);
	void (*set_reloadMode)(struct Common_Timer* commom_timer, const UBaseType_t uxAutoReload);
	void* (*get_timer_id)(struct Common_Timer* commom_timer);
}Common_Timer;

void commom_timer_init(Common_Timer* commom_timer);