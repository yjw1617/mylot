#include "dev_handle.h"
#include "common_dev.h"
#include "leinuo_dev.h"
#include "common_dev.h"
#include "mygui.h"
static dev_controller* dev_con;
//循环遍历各种dev设备的消息队列处理相关事件
void dev_handle(){
	leinuo_dev_init();//初始化leinuo设备
	mygui_init();			//初始化mygui设备
	dev_poll_handle();
}

void dev_poll_handle(){
	//获取dev_con句柄
	dev_controller* dev_con = common_dev_get_controller();
	BaseType_t ret = 0;
	Frame_t frame_temp = {};
	for(;;){
		for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
				//接收dev的消息队列
				if(dev_con->dev[i] != NULL){
//					LOG("kk dev_addr = %d\r\n", dev_con.dev[i]->addr);
					if(dev_con->dev[i]->Message_Queue != NULL){
//						LOG("hh dev_addr = %d\r\n", dev_con.dev[i]->addr);
//						LOG("kk i = %d\r\n", i);
						if(dev_con->dev[i]->ops->msg_parse != NULL){
//							LOG("qq dev_addr = %d\r\n", dev_con.dev[i]->addr);
							ret = xQueueReceive(dev_con->dev[i]->Message_Queue, &frame_temp ,0);
							if(ret == pdPASS){
								for(uint8_t i = frame_temp.index_useful; i < frame_temp.len; i++){
									LOG("%.2x ", frame_temp.r_buf[i]);
								}
								
								message_log(dev_con->dev[i]);
								//将消息传入dev设备的解析函数
								dev_con->dev[i]->ops->msg_parse(dev_con->dev[i]->mydev, &frame_temp.r_buf[frame_temp.index_useful], frame_temp.len);
							}
						}
					}
				}
		}
		vTaskDelay(1);
	}
}