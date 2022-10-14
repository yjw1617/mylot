#include "common_dev.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "message.h"
static dev_controller dev_con = {0};
void* dev_mydev_create(uint16_t size){
	return pvPortMalloc(size);
}

int8_t dev_add(Dev* dev){
	for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
		if(dev_con.dev[i] == 0){
			dev_con.dev[i] = dev;
			dev_con.num++;
			LOG("dev_register success\r\n");
			LOG("addr = %d\r\n", dev->addr);
			return pdTRUE;
		}
	}
	LOG("dev_register buf full\r\n");
	return pdFALSE;
}

uint8_t unregister_dev(Dev* dev){
	for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
		if(dev == dev_con.dev[i]){
			dev_con.dev[i] = 0;
		}
	}
}

uint8_t dev_del(Dev* dev){
	vPortFree(dev);
}


Dev* dev_find_dev_by_addr(uint16_t addr){
	for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
		if(dev_con.dev[i]->addr == addr){
			return dev_con.dev[i];
		}
	}
	LOG("dev_find_dev_by_addr fail\r\n");
	return NULL;
}


Dev* dev_find_dev_by_name(uint8_t* dev_name){
	for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
		if(memcmp(dev_name, dev_con.dev[i]->name, strlen((char*)dev_name))){
			return dev_con.dev[i];
		}
	}
	LOG("dev_find_dev_by_name\r\n");
	return NULL;
}

uint8_t dev_find_protocoltype_by_addr(uint16_t addr){
	for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
		if(dev_con.dev[i]->addr == addr){
			return dev_con.dev[i]->protocol_type;
		}
	}
}

void common_dev_init(){
	memset(&dev_con, 0, sizeof(dev_controller));
	
}
void dev_poll_handle(){
	BaseType_t ret = 0;
	Frame_t frame_temp = {};
	for(;;){
		for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
				//接收dev的消息队列
				if(dev_con.dev[i] != NULL){
//					LOG("kk dev_addr = %d\r\n", dev_con.dev[i]->addr);
					if(dev_con.dev[i]->Message_Queue != NULL){
//						LOG("hh dev_addr = %d\r\n", dev_con.dev[i]->addr);
//						LOG("kk i = %d\r\n", i);
						if(dev_con.dev[i]->ops->msg_parse != NULL){
//							LOG("qq dev_addr = %d\r\n", dev_con.dev[i]->addr);
							ret = xQueueReceive(dev_con.dev[i]->Message_Queue, &frame_temp ,0);
							if(ret == pdPASS){
								//将消息传入dev设备的解析函数
								dev_con.dev[i]->ops->msg_parse(dev_con.dev[i], &frame_temp.r_buf[frame_temp.index_useful], frame_temp.len);
							}
						}
					}
				}
		}
		vTaskDelay(1);
	}
}

void dev_init(Dev* dev, operations* opts){
	dev->ops = opts;
}





