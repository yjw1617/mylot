#include "dev_handle.h"
#include "common_dev.h"
#include "leinuo_driver.h"
#include "common_dev.h"
#include "mygui_driver.h"
#include "uart_driver.h"
#include "common_event.h"
#include "fingerprint_driver.h"
#include <stdio.h>
static dev_controller* dev_con;
//驱动初始化函数
void dev_handle(){
	common_event_init();
	leinuo_dev_init();//初始化leinuo设备
	mygui_init();			//初始化mygui设备
	fingerprint_dev_init();
//	uart_dev_init();	//初始化uart设备
//	dev_poll_handle();
}
//循环遍历各种dev设备的消息队列处理相关事件
//void dev_poll_handle(){
//	//获取dev_con句柄
//	dev_controller* dev_con = common_dev_get_controller();
//	BaseType_t ret = 0;
//	Frame_t frame_temp = {};
//	QueueHandle_t event_queue= common_get_event_queue();
//	Common_Event event = {0};
//	for(;;){
//		ret = xQueueReceive(event_queue, &event ,portMAX_DELAY);
//		if(ret == pdPASS){
//			if(event.handle != NULL){
//				event.handle(event.handle_args, event.type, event.id, event.data, event.data_len);
//			}else{
//				A_Log("event.handle == NULL\r\n");
//			}
//		}
//	}
//}