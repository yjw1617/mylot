#include "common_event.h"
#include "common_timer.h"
#include <stdio.h>
#include <string.h>
#define Common_Event_Max_Num 100
#define Common_Timer_Temp_Events_Data_Len 20
typedef struct Timer_temp_event{
	uint32_t timer_ticks_delay;
	int16_t timer_id;
	uint16_t loop_num;
	uint8_t reload_mode;
	Common_Event events;
	uint8_t timer_temp_events_data[Common_Timer_Temp_Events_Data_Len];
}Timer_temp_event;

typedef struct Common_Events_Control{
	uint16_t register_total_num;
	uint16_t post_total_num;
	QueueHandle_t Common_Event_Queue;
	Common_Event* events[Common_Event_Max_Num];
	
	Timer_temp_event timer_temp_events[Common_Event_Max_Num];
	SemaphoreHandle_t mutex;
	Common_Timer timer[Common_Event_Max_Num];
}Common_Events_Control;

static Common_Events_Control event_con = {};
	

static Common_Event* common_find_event_by_timer_id(int16_t timer_id){
	for(uint8_t i = 0 ; i < Common_Event_Max_Num; i++){
		if(event_con.timer_temp_events[i].timer_id == timer_id){
			//返回event
			return &event_con.timer_temp_events[i].events;
		}
	}
	return NULL;
}
//通过timerid找到与之对应的Timer_temp_event
static Timer_temp_event* common_find_temp_event_by_timerId(TimerHandle_t xTimer){
	for(uint8_t i = 0 ; i < Common_Event_Max_Num; i++){
		if(event_con.timer_temp_events[i].timer_id == *(uint32_t*)pvTimerGetTimerID(xTimer)){
			Error_Check(NULL, event_con.timer_temp_events[i].events.handle);
			if(event_con.timer_temp_events[i].events.handle != NULL){
				return &event_con.timer_temp_events[i];
			}
		}
	}
	return NULL;
}

static void timer_callback(TimerHandle_t xTimer){
	LOG("\r\n---leinuo timer callback---\r\n");
	Timer_temp_event* temp_event = common_find_temp_event_by_timerId(xTimer);
	if(temp_event != NULL){
		LOG("pop post_total_num = %d\r\n", event_con.post_total_num);
	}
	Error_Check(NULL, temp_event);
	//运行回调函数
	temp_event->events.handle(temp_event->events.handle_args, temp_event->events.type, temp_event->events.id, temp_event->events.data, temp_event->events.data_len);
	if(temp_event->reload_mode == 0){//有限次数
		event_con.post_total_num--;
		temp_event->timer_id = -1;//必须给-1表示这个timer可用
		memset(&temp_event->events, 0, sizeof(Common_Event));//将存储事件清空
		memset(&temp_event->timer_temp_events_data, 0, Common_Timer_Temp_Events_Data_Len);
	}
}
	
void common_event_init(){
	event_con.Common_Event_Queue = xQueueCreate(Common_Event_Max_Num, sizeof(Common_Event));
	Error_Check(NULL, event_con.Common_Event_Queue);
	event_con.mutex = xSemaphoreCreateMutex();
	Error_Check(NULL, event_con.mutex);
	
	for(uint8_t i = 0 ; i < Common_Event_Max_Num; i++){
		event_con.timer[i].autoReload = 0;
		event_con.timer[i].id = i;
		event_con.timer[i].period = 10000;
		event_con.timer[i].callback_function = timer_callback;
		event_con.timer[i].ticksToWait = 1000;
		commom_timer_init(&event_con.timer[i]);
		event_con.timer[i].create(&event_con.timer[i]);
		
		event_con.timer_temp_events[i].timer_id = -1;
	}
}

//获得可用的定时器
static Common_Timer* common_get_available_timer(){
	for(uint8_t i = 0 ; i < Common_Event_Max_Num; i++){
		if(pdFALSE == xTimerIsTimerActive(event_con.timer[i].handle)){//The timer is not running.
			return &event_con.timer[i];
		}
	}
	return NULL;
}

static int8_t common_save_temp_event(int16_t timer_id, uint32_t timer_ticks_delay, uint8_t reload_mode, Common_Event* event){
	Error_Check(1, (event->data_len > Common_Timer_Temp_Events_Data_Len));
	for(uint8_t i = 0 ; i < Common_Event_Max_Num; i++){
		if(event_con.timer_temp_events[i].timer_id == -1){
			event_con.timer_temp_events[i].timer_id = timer_id;
			event_con.timer_temp_events[i].timer_ticks_delay = timer_ticks_delay;
			event_con.timer_temp_events[i].reload_mode = reload_mode;
			memcpy(&event_con.timer_temp_events[i].events, event, sizeof(Common_Event));
			memcpy((void*)event_con.timer_temp_events[i].timer_temp_events_data, event->data, event->data_len);
			return 0;
		}
	}
	return -1;
}

int8_t common_event_handler_register(uint16_t event_type, uint32_t event_id, event_handler_t handle, void* event_handler_arg){
	Error_Check(0, handle);
	for(uint16_t i = 0; i < Common_Event_Max_Num; i++){
		if(event_con.events[i] == 0){
			//创建一个event
			Common_Event* event = (Common_Event*)pvPortMalloc(sizeof(Common_Event));
			if(event == NULL){
				LOG("event_register success error---");
				return -1;
			}
			event_con.events[i] = event;
			event_con.events[i]->id = event_id;
			event_con.events[i]->type = event_type;
			event_con.events[i]->handle = handle;
			event_con.events[i]->handle_args = event_handler_arg;
			event_con.register_total_num++;
			LOG("event_register success\r\n");
			return pdTRUE;
		}
	}
	LOG("event_register success error");
	return -1;
}


int8_t common_event_post(uint16_t event_type, uint32_t event_id, void* event_data, uint16_t event_data_lens, uint32_t ticks_to_wait, uint8_t reload_mode){
	//将事件加入事件队列
	int8_t ret = 0;
	uint32_t timer_ticks_delay = 0;
	int8_t loop_num_tmp = 0;
	Common_Timer* timer = NULL;
	for(uint16_t i = 0; i < Common_Event_Max_Num; i++){
		if(event_con.events[i] != NULL){
			if(event_con.events[i]->type == event_type && event_con.events[i]->id == event_id){
				event_con.events[i]->data = event_data;
				event_con.events[i]->data_len = event_data_lens;
				if(event_con.events[i]->handle != NULL){
					if(ticks_to_wait == 0){
						event_con.events[i]->handle(event_con.events[i]->handle_args, event_con.events[i]->type, event_con.events[i]->id, event_con.events[i]->data, event_con.events[i]->data_len);
						return 0;
					}
					//由定时器去处理事件
					timer = common_get_available_timer();
					Error_Check(NULL, timer);
					while(timer == NULL){
						timer = common_get_available_timer();
						vTaskDelay(1);
					}
					timer->set_reloadMode(timer, reload_mode);
					timer->change_period(timer, ticks_to_wait);
					//将事件暂存在一个事件
					while(common_save_temp_event(timer->get_timer_id(timer), ticks_to_wait, reload_mode, event_con.events[i]) == -1){
						vTaskDelay(1);
					}
					event_con.post_total_num++;
					LOG("post event_con.post_total_num = %d\r\n", event_con.post_total_num);
					timer->start(timer);
					return 0;
				}
				LOG("event_con.events[i]->handle error\r\n");
				return -1;
			}
		}
	}
	LOG("event_con.events[i] is not found\r\n");
	return -1;
}

QueueHandle_t common_get_event_queue(){
	return event_con.Common_Event_Queue;
}




	
	


