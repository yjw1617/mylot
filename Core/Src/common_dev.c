#include "common_dev.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "message.h"
static dev_controller dev_con = {0};
void* common_mydev_create(uint16_t size){
	return pvPortMalloc(size);
}

int8_t common_dev_register(Dev* dev){
	for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
		if(dev_con.dev[i] == 0){
			dev_con.dev[i] = dev;
			dev_con.num++;
			LOG("dev_register success\r\n");
			LOG("addr = %d\r\n", dev->id);
			return pdTRUE;
		}
	}
	LOG("dev_register buf full\r\n");
	return pdFALSE;
}

uint8_t common_dev_unregister(Dev* dev){
	for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
		if(dev == dev_con.dev[i]){
			dev_con.dev[i] = 0;
		}
	}
}

uint8_t common_dev_del(Dev* dev){
	vPortFree(dev);
}


Dev* common_dev_find_dev_by_id(uint16_t addr){
	for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
		if(dev_con.dev[i] != NULL){
			if(dev_con.dev[i]->id == addr){
				return dev_con.dev[i];
			}
		}
	}
	LOG("common_dev_find_dev_by_id fail\r\n");
	return NULL;
}


Dev* common_dev_find_dev_by_name(uint8_t* dev_name){
	for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
		if(dev_con.dev[i] != NULL){
			if(memcmp(dev_name, dev_con.dev[i]->name, strlen((char*)dev_name)) == 0){
				return dev_con.dev[i];
			}
		}
	}
	LOG("common_dev_find_dev_by_name\r\n");
	return NULL;
}

void common_dev_init(){
	memset(&dev_con, 0, sizeof(dev_controller));
	
}

dev_controller* common_dev_get_controller(){
	return &dev_con;
}

void common_dev_opts_init(Dev* dev, operations* opts){
	dev->ops = opts;
}

int8_t common_open(uint8_t* dev_name){
	for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
		if(dev_con.dev[i] != NULL){
			if(memcmp(dev_name, dev_con.dev[i]->name, strlen((char*)dev_name)) == 0){
				if(dev_con.dev[i]->ops->open != NULL){
					dev_con.dev[i]->ops->open(dev_con.dev[i]->mydev);
				}
				return dev_con.dev[i]->id;//返回设备id
			}
		}
	}
	return -1;
}

int8_t common_write(uint16_t dev_id, void* buf, uint32_t len){
	for(uint16_t i = 0; i < DEV_MAX_NUM; i++){
		if(dev_con.dev[i] != NULL){
			if(dev_con.dev[i]->id == dev_id){
				if(dev_con.dev[i]->ops->write != NULL){
					LOG("dev_con.dev\r\n");
					return dev_con.dev[i]->ops->write(dev_con.dev[i]->mydev, buf, len);
				}
			}
		}
	}
	return -1;
}









