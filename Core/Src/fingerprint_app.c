#include "fingerprint_app.h"
#include "common_dev.h"
#include "common_event.h"
#include "message.h"
#include <stdio.h>
#include <string.h>
typedef struct Fingerprint_app_t{
	QueueHandle_t queue;
	uint16_t temp_event_id;
}Fingerprint_app_t;

typedef struct fingerprint_t{
	int8_t fd;
	uint8_t status;
}fingerprint_t;


static Fingerprint_app_t fingerprint_app;
static fingerprint_t fingerprint;


static int8_t write_fd_event(int8_t fd, uint16_t event_type, uint16_t event_id, uint8_t* buf, uint16_t buf_len);
//驱动函数来回调结果通过消息队列来通知到应用层
static void event_handler(void *event_handler_arg, uint16_t event_type, int32_t event_id, void* event_data, uint16_t event_data_len){
	switch(event_type){
		case Event_Type_Fingerprint:
				switch(event_id){
					case Event_Id_Fingerprint_Test://指纹模块是否被按检测
						if(memcmp(event_data, "run", strlen("run")) == 0){
							write_fd_event(fingerprint.fd, Event_Type_Fingerprint, Event_Id_Fingerprint_Test, "run", sizeof("run"));
						}
						if(memcmp(event_data, "touched", strlen("touched")) == 0){//如果模块被手指按下
							write_fd_event(fingerprint.fd, Event_Type_Fingerprint, Event_Id_Fingerprint_Power, "on", strlen("on"));//供电
							//140ms 时间进行初始化工作
							A_Log("delay 300ms");
							vTaskDelay(300);
							//进行自动验证指纹
							write_fd_event(fingerprint.fd, Event_Type_Fingerprint, Event_Id_Fingerprint_AutoIdentify, NULL, 0);
							A_Log("Event_Id_Fingerprint_AutoIdentify!!");
						}
						break;
					case Event_Id_Fingerprint_Power://模块电源管理
						if(memcmp(event_data, "on", strlen("on"))){
							A_Log("Event_Id_Fingerprint_Power on");
							write_fd_event(fingerprint.fd, Event_Type_Fingerprint, Event_Id_Fingerprint_Power, "on", sizeof("on"));
						}
						if(memcmp(event_data, "off", strlen("off"))){
							
						}
						break;
					case Event_Id_Fingerprint_AutoIdentify://自动验证指纹
						A_Log("Event_Id_Fingerprint_AutoIdentify!!");
						write_fd_event(fingerprint.fd, Event_Type_Fingerprint, Event_Id_Fingerprint_AutoIdentify, NULL, 0);
						break;
				}
			break;			
	}
}
static int8_t write_fd_event(int8_t fd, uint16_t event_type, uint16_t event_id, uint8_t* buf, uint16_t buf_len){
	Common_Event event1 = {
			.type = event_type,
			.id = event_id,
			.data = buf,
			.data_len = buf_len,
	};
	A_Log("fp write\r\n");
	Error_Check(-1, common_write(fd, &event1, sizeof(event1)));
	return 0;
}

QueueHandle_t app_get_fingerprintApp_queue(){
	return fingerprint_app.queue;
}

int fingerprint_app_main(){
		fingerprint_app.queue = xQueueCreate(3, sizeof(Common_Event));
		//注册事件
		Error_Check(-1, common_event_handler_register(Event_Type_Fingerprint, Event_Id_Fingerprint_Test, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Fingerprint, Event_Id_Fingerprint_Power, event_handler, NULL));
		Error_Check(-1, common_event_handler_register(Event_Type_Fingerprint, Event_Id_Fingerprint_AutoIdentify, event_handler, NULL));
		fingerprint.fd = common_open((uint8_t*)"fingerprint1");
		A_Log("fingerprint.fd = %d\r\n", fingerprint.fd);
		Error_Check(-1, fingerprint.fd);
		uint8_t buf[25] = {0};
		BaseType_t ret = 0;
		
		Common_Event app_event = {0};
		Common_Payload payload = {0};
		for(;;){
			ret = xQueueReceive(fingerprint_app.queue, &app_event, sizeof(Common_Event));
			if(ret == pdPASS){
				switch(app_event.type){
					case Event_Type_Fingerprint:
							switch(app_event.id){
								case Event_Id_Fingerprint_Test:
									if(memcmp(app_event.data, "start", strlen("start")) == 0){
										payload.reload = 1;
										payload.delay_ticks = 1000;
										memcpy(payload.data, "start", strlen("start"));
										write_fd_event(fingerprint.fd, Event_Type_Fingerprint, Event_Id_Fingerprint_Test, (uint8_t*)&payload, sizeof(payload));
										A_Log("Event_Id_Fingerprint_Test start\r\n");
									}
									break;
								case Event_Id_Fingerprint_Power:
									A_Log("Event_Id_Fingerprint_Power\r\n");
									break;
								case Event_Id_Fingerprint_AutoIdentify:
									A_Log("Event_Id_Fingerprint_AutoIdentify\r\n");
									break;
							}
						break;			
				}
				//销毁r_buf
				common_event_destroy(&app_event);
				memset(&app_event, 0, sizeof(Common_Event));
				memset(&payload, 0, sizeof(Common_Payload));
			}
		}
}