#include "common_event.h"
#include "common_timer.h"
#include <stdio.h>
#include <string.h>
#define Common_Event_Max_Num 100
#define Common_Timer_Temp_Events_Data_Len 20
#define Common_Timer_Create(...) xTimerCreate(__VA_ARGS__)
#define Common_Timer_Start(...)	xTimerStart(__VA_ARGS__)
typedef struct Timer_temp_event{
	uint8_t reload_mode;
	Common_Event events;
	uint8_t timer_temp_events_data[Common_Timer_Temp_Events_Data_Len];
	TimerHandle_t timer;//按需创建
}Timer_temp_event;

typedef struct Common_Events_Control{
	uint16_t register_total_num;
	uint16_t post_total_num;
	Common_Event* events[Common_Event_Max_Num];
	Timer_temp_event* timer_temp_events[Common_Event_Max_Num];//按需创建
	SemaphoreHandle_t mutex;
}Common_Events_Control;

static Common_Events_Control event_con = {};
	
//通过timerid找到与之对应的Timer_temp_event
static int16_t common_find_temp_event_id_by_TimerHandle(TimerHandle_t xTimer){
	for(uint16_t i = 0 ; i < Common_Event_Max_Num; i++){
		if(event_con.timer_temp_events[i]->timer == xTimer){
			Error_Check(NULL, event_con.timer_temp_events[i]->events.handle);
			if(event_con.timer_temp_events[i]->events.handle != NULL){
				return i;
			}
		}
	}
	return -1;
}

static void timer_callback(TimerHandle_t xTimer){
	K_Log("event timer callback");
	int16_t temp_event_id = common_find_temp_event_id_by_TimerHandle(xTimer);
	K_Log("take temp_event_id = %d", temp_event_id);
	K_Log("type = %d   id = %d   data = %s", event_con.timer_temp_events[temp_event_id]->events.type, event_con.timer_temp_events[temp_event_id]->events.id, event_con.timer_temp_events[temp_event_id]->events.data);
	Error_Check(-1, temp_event_id);
	event_con.timer_temp_events[temp_event_id]->events.handle(event_con.timer_temp_events[temp_event_id]->events.handle_args, event_con.timer_temp_events[temp_event_id]->events.type, event_con.timer_temp_events[temp_event_id]->events.id, event_con.timer_temp_events[temp_event_id]->events.data, event_con.timer_temp_events[temp_event_id]->events.data_len);
	if(event_con.timer_temp_events[temp_event_id]->reload_mode == 0){//1次
		//将timer和Timer_temp_event删除
		Error_Check(pdFAIL, xTimerDelete(xTimer, portMAX_DELAY));
		if(event_con.timer_temp_events[temp_event_id] != NULL){
			K_Log("timer_callback del event_type = %d , event_id = %d---", event_con.timer_temp_events[temp_event_id]->events.type, event_con.timer_temp_events[temp_event_id]->events.id);
			vPortFree(event_con.timer_temp_events[temp_event_id]);
			event_con.timer_temp_events[temp_event_id] = NULL;
			event_con.post_total_num--;
		}
	}
}
	
void common_event_init(){
	event_con.mutex = xSemaphoreCreateMutex();
	Error_Check(NULL, event_con.mutex);
}

//将post过来的event事件暂存到暂存事件库中用定时器处理
static int16_t common_add_temp_event(Common_Event* event, uint8_t reload_mode, uint32_t timer_ticks_delay, TimerCallbackFunction_t call_back){
	Error_Check(1, (event->data_len > Common_Timer_Temp_Events_Data_Len));
	for(uint16_t i = 0 ; i < Common_Event_Max_Num; i++){
		if(event_con.timer_temp_events[i] == NULL){
			K_Log("create temp_event_id = %d", i);
			//创建一个事件
			event_con.timer_temp_events[i] = pvPortMalloc(sizeof(Timer_temp_event));
			Error_Check(NULL, event_con.timer_temp_events[i]);
			K_Log("pvPortMalloc(sizeof(Timer_temp_event)) suc");
			memcpy(&event_con.timer_temp_events[i]->events, event, sizeof(Common_Event));
			memcpy((void*)event_con.timer_temp_events[i]->timer_temp_events_data, event->data, event->data_len);
			event_con.timer_temp_events[i]->reload_mode = reload_mode;
			//创建一个定时器
			event_con.timer_temp_events[i]->timer = Common_Timer_Create(NULL, timer_ticks_delay, reload_mode, &i, call_back);
			Error_Check(NULL, event_con.timer_temp_events[i]->timer);
			return i;
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
				K_Log("event_register success error");
				return -1;
			}
			event_con.events[i] = event;
			event_con.events[i]->id = event_id;
			event_con.events[i]->type = event_type;
			event_con.events[i]->handle = handle;
			event_con.events[i]->handle_args = event_handler_arg;
			event_con.register_total_num++;
			K_Log("event_register success");
			return pdTRUE;
		}
	}
	K_Log("event_register success error");
	return -1;
}


int8_t common_event_post(uint16_t event_type, uint32_t event_id, void* event_data, uint16_t event_data_lens, uint32_t ticks_to_wait, uint8_t reload_mode){
	//将事件加入事件队列
	int8_t ret = 0;
	uint32_t timer_ticks_delay = 0;
	int8_t loop_num_tmp = 0;
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
					//将event_con.events[i]存到event_con.timer_temp_events中
					int16_t temp_event_id = common_add_temp_event(event_con.events[i], reload_mode, ticks_to_wait, timer_callback);
					Error_Check(-1, temp_event_id);
					event_con.post_total_num++;
					K_Log("post event_con.post_total_num = %d", event_con.post_total_num);
					Common_Timer_Start(event_con.timer_temp_events[temp_event_id]->timer, portMAX_DELAY);
					return temp_event_id;
				}
				K_Log("event_con.events[i]->handle error");
				return -1;
			}
		}
	}
	K_Log("event_con.events[i] is not found");
	return -1;
}

//将所有匹配的事件全部删除
int8_t common_del_temp_event_by_eventType_and_eventId(uint16_t event_type, uint32_t event_id){
	for(uint16_t i = 0; i < Common_Event_Max_Num; i++){
		if(event_con.timer_temp_events[i] != NULL){
			if(event_con.timer_temp_events[i]->events.type == event_type && event_con.timer_temp_events[i]->events.id == event_id){
				if(event_con.timer_temp_events[i]->timer != NULL){
					//将改temp_events资源删除
					Error_Check(pdFAIL, xTimerDelete(event_con.timer_temp_events[i]->timer, portMAX_DELAY));
				}
				K_Log("common_del del event_type = %d , event_id = %d", event_con.timer_temp_events[i]->events.type, event_con.timer_temp_events[i]->events.id);
				vPortFree(event_con.timer_temp_events[i]);
				event_con.timer_temp_events[i] = NULL;
				event_con.post_total_num--;
			}
		}
	}
	return 0;
}

int8_t common_event_destroy(Common_Event* event){
	vPortFree(event->data);
}

int8_t common_write_queue_event(QueueHandle_t queue, uint16_t event_type, uint16_t event_id, uint8_t* buf, uint16_t buf_len){
	//拷贝buf内容
	uint8_t* r_buf = pvPortMalloc(buf_len);
	memcpy(r_buf, buf, buf_len);
	Common_Event event = {
			.type = event_type,
			.id = event_id,
			.data = r_buf,
			.data_len = buf_len,
	};
	return xQueueSend(queue, &event, portMAX_DELAY);
}



	
	


