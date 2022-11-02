#pragma once
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#define DEV_MAX_NUM 10 //设备库的最大设备数目
#define DEV_NAME_MAX_LEN 20 //设备名最大长度
typedef struct operations{
	int8_t (*write)(void* mydev, void* data, uint32_t len);
	int8_t (*open)(void* mydev);
	int8_t (*on)(void*);
	int8_t (*off)(void*);
	int8_t (*reset)(void*);
	int8_t (*connect)(void*);
	int8_t (*ioctl)(void* my_dev, uint16_t cmd, uint32_t arg, uint16_t len);
	uint8_t (*uart_msg_recv)(void* my_dev, uint8_t* buf, uint8_t len);
}operations;	

typedef struct Dev {
	uint16_t id;
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

Dev* common_dev_find_dev_by_id(uint16_t addr);

void common_dev_opts_init(Dev* dev, operations* opts);

int8_t common_dev_register(Dev* dev);

dev_controller* common_dev_get_controller();

void dev_poll_handle();

int8_t common_open(uint8_t* dev_name);

int8_t common_write(uint16_t dev_id, void* buf, uint32_t len);

