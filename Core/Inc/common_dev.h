#pragma once
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#define DEV_MAX_NUM 10 //设备库的最大设备数目
#define DEV_NAME_MAX_LEN 20 //设备名最大长度
typedef struct operations{
	uint8_t (*on)(void*);
	uint8_t (*off)(void*);
	uint8_t (*reset)(void*);
	uint8_t (*connect)(void*);
	uint8_t (*ioctl)(void* my_dev, uint16_t cmd, uint32_t arg, uint16_t len);
	uint8_t (*msg_parse)(void* my_dev, uint8_t* buf, uint8_t len);
}operations;	

typedef struct Dev {
	uint16_t addr;
	uint8_t protocol_type;
	uint8_t name[DEV_NAME_MAX_LEN];
	QueueHandle_t Message_Queue;
	void* mydev;//指向自己的子对象
	struct operations *ops;
}Dev ;

typedef struct dev_controller{
	uint16_t num;
	Dev* dev[DEV_MAX_NUM];
}dev_controller;
void* common_mydev_create(uint16_t size);

uint8_t common_dev_del(Dev* dev);

int8_t common_dev_register(Dev* dev);

Dev* common_dev_find_dev_by_name(uint8_t* dev_name);

Dev* common_dev_find_dev_by_addr(uint16_t addr);

void common_dev_opts_init(Dev* dev, operations* opts);

int8_t common_dev_register(Dev* dev);

dev_controller* common_dev_get_controller();

void dev_poll_handle();

