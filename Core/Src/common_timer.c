#include "common_timer.h"
#include "message.h"
static BaseType_t start(Common_Timer* commom_timer){
	xTimerStart(commom_timer->handle, pdMS_TO_TICKS(commom_timer->ticksToWait));
}

static BaseType_t create(Common_Timer* commom_timer){
	commom_timer->handle = xTimerCreate((char*)commom_timer->name, commom_timer->period, commom_timer->autoReload, &commom_timer->id, commom_timer->callback_function);
	if(!commom_timer->handle){
		A_Log("\r\n xTimerCreate fail !!!\r\n");
	}
}

static BaseType_t change_period(Common_Timer* commom_timer, TickType_t period){
	commom_timer->period = period;
	xTimerChangePeriod(commom_timer->handle, commom_timer->period, pdMS_TO_TICKS(commom_timer->ticksToWait));
}

static BaseType_t Delete(Common_Timer* commom_timer){
	xTimerDelete(commom_timer->handle, pdMS_TO_TICKS(commom_timer->ticksToWait));
}

static BaseType_t stop(Common_Timer* commom_timer){
	xTimerStop(commom_timer->handle, pdMS_TO_TICKS(commom_timer->ticksToWait));
}

static void set_reloadMode(Common_Timer* commom_timer, const UBaseType_t uxAutoReload ){
	(uxAutoReload == 1)? (commom_timer->autoReload = 1) : (commom_timer->autoReload = 0);
	vTimerSetReloadMode(commom_timer->handle, uxAutoReload);
}

static uint32_t get_timer_id(Common_Timer* commom_timer){
	return *(uint32_t*)pvTimerGetTimerID(commom_timer->handle);
}
void commom_timer_init(Common_Timer* commom_timer){
	commom_timer->start = start;
	commom_timer->create = create;
	commom_timer->Delete = Delete;
	commom_timer->change_period = change_period;
	commom_timer->stop = stop;
	commom_timer->set_reloadMode = set_reloadMode;
	commom_timer->get_timer_id = get_timer_id;
	
}