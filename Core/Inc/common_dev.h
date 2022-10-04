#pragma once
#include <stdint.h>
#define DEV_MAX_NUM 10 //设备库的最大设备数目
#define DEV_NAME_MAX_LEN 20
typedef struct operations{
	uint8_t (*on)(void*);
	uint8_t (*off)(void*);
	uint8_t (*reset)(void*);
	uint8_t (*connect)(void*);
	uint8_t (*ioctl)(void* my_dev, uint16_t cmd, uint32_t arg);
}operations;	

typedef struct Dev {
	uint16_t addr;
	uint8_t name[DEV_NAME_MAX_LEN];
	void* mydev;//指向自己的子对象
	struct operations *ops;
}Dev ;

typedef struct dev_controller{
	uint16_t num;
	Dev* dev[DEV_MAX_NUM];
}dev_controller;
void* dev_mydev_create(uint16_t size);

uint8_t dev_del(Dev* dev);

int8_t dev_register(Dev* dev);

Dev* dev_find_dev_by_name(uint8_t* dev_name);

Dev* dev_find_dev_by_addr(uint16_t addr);


