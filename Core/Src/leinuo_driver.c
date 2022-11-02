#include "leinuo_driver.h"
#include "message.h"
#include "string.h"
#include "usart.h"
#include "utils.h"
#include <stdio.h>
#include "common_event.h"
#include "FreeRTOS.h"
#include "semphr.h"
#define LeiNuoWifi_Dev_Max_NUM 3
static LeiNuoWifi_dev* pg_mydev[LeiNuoWifi_Dev_Max_NUM];

static uint8_t connect(void* my_dev){
	LOG("wifi connect\r\n");
}

static uint8_t on(void* my_dev){
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	xSemaphoreTake(mydev->mutex, portMAX_DELAY);
	xSemaphoreGive(mydev->mutex);
}

static uint8_t off(void* my_dev){
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	xSemaphoreTake(mydev->mutex, portMAX_DELAY);
	LOG("wifi %s off\r\n", mydev->dev.name);
	xSemaphoreGive(mydev->mutex);
}

static uint8_t reset(void* my_dev){
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	xSemaphoreTake(mydev->mutex, portMAX_DELAY);
	LOG("wifi reset\r\n");
	xSemaphoreGive(mydev->mutex);
}

static uint8_t ioctl(void* my_dev, uint16_t cmd, uint32_t arg, uint16_t len){
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	uint8_t* payload = 0;
	if(len > 0){
		payload = (uint8_t*)arg;
	}
}
static void leinuo_make_mes(Message_Leinuo_t* mes, uint8_t* buf){
	buf[0] = mes->head1;
	buf[1] = mes->head2;
	buf[2] = mes->addr_src;
	buf[3] = mes->addr_dest;
	buf[4] = mes->type;
	buf[5] = mes->cmd;
	buf[6] = mes->len;
	memcpy(&buf[7], mes->payload, mes->len);
	buf[7 + mes->len] = message_get_checknum(buf, mes->len + 8);
}
	
static void timer_callback(TimerHandle_t xTimer){
	LOG("leinuo timer callback\r\n");
}

static int8_t leinuo_wake_up(void* my_dev){
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	Dev* p_dev = common_dev_find_dev_by_name("uart1");
	if(p_dev == NULL){
		LOG("common_dev_find_dev_by_name('uart1') error\r\n");
		return -1;
	}
	Message_Leinuo_t leinuo_mes = {
		.addr_src = Message_Addr_Wifi_LEINUO1,
		.addr_dest = Message_Addr_Wifi_LEINUO1,
		.type = 0x01,
		.cmd = Message_Uart_Send,
		.payload = {0xaa, 0x33, 0x11, 0x22, 0x33, 0x44},
		.len = 6,
	};
	message_send_to_dev(p_dev, (uint8_t*)&leinuo_mes, Protocol_Type_Mcu);
}

/*一个设备可以支持多个类型的消息协议*/
static uint8_t leinuo_uart_msg_recv(void* my_dev, uint8_t* buf, uint8_t len){
	if(my_dev == NULL){
		LOG("msg_parse my_dev is null\r\n");
		return -1;
	}
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	if(strncmp(buf, "leinuo1 connect suc", len) == 0){
		if(strncmp(mydev->dev.name, "LeiNuoWifi1", strlen("LeiNuoWifi1")) == 0){
			common_event_post(Event_Type_Wifi, Event_Id_Connect_Suc, "connect suc", strlen("connect suc"), 1, 1);
		}
	}
}

static int8_t leinuo_connct(){
	LOG("leinuo1 connect\r\n");
//	common_event_post(Event_Type_Wifi, Event_Id_Connect_Failed, NULL, 0, 8000, 0);
}
	
static int8_t write(void* my_dev, void* data, uint32_t len){
	if(my_dev == NULL){
		LOG("msg_parse my_dev is null\r\n");
		return -1;
	}
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	LOG("%s write\r\n", mydev->dev.name);
	//解析数据
	Common_Event* event = (Common_Event*)data;
	int8_t ret = -1;
	switch(event->type){
		case Event_Type_Wifi:
			switch(event->id){
				case Event_Id_Connect:
					Error_Check(-1, leinuo_connct());
					break;
				case Event_Id_Wakeup:
					break;
				case Event_Id_Reset:
					break;
			}
			break;
		default:
			LOG("illegal event\r\n");
			break;
	}
	
}

static operations opts = {
//	.msg_parse = msg_parse,
	.write = write,
	.uart_msg_recv = leinuo_uart_msg_recv,
};


operations* leinuo_get_mydev_p(){
	return &opts;
}

static void leinuo_timer_init(LeiNuoWifi_dev* mydev){
	//定时器初始化
	memcpy(mydev->timer.name, "LeiNuoWifi1_timer", strlen((char*)"LeiNuoWifi1_timer"));
	mydev->timer.autoReload = pdTRUE;
	mydev->timer.id = 0;
	mydev->timer.period = 10000;
	mydev->timer.callback_function = timer_callback;
	mydev->timer.ticksToWait = 1000;
	commom_timer_init(&mydev->timer);
}

int8_t leinuo_dev_init(){
	int8_t ret = 0;
	for(uint8_t i = 0; i < LeiNuoWifi_Dev_Max_NUM; i++){
		pg_mydev[i]= common_mydev_create(sizeof(LeiNuoWifi_dev));//创建雷诺结构体
		if(pg_mydev[i] == NULL){
			LOG("dev_mydev_%d create LeiNuoWifi_dev error\r\n", i);
			return -1;
		}
		memset(pg_mydev[i], 0, sizeof(LeiNuoWifi_dev));//初始化leino结构体为0
		common_dev_opts_init(&pg_mydev[i]->dev, &opts);//绑定操作函数
		pg_mydev[i]->dev.mydev = pg_mydev[i];//将自己的mydev指针指向子类
		pg_mydev[i]->dev.Message_Queue = xQueueCreate(2 , sizeof(Frame_t));
		if(pg_mydev[i]->dev.Message_Queue == NULL){
			LOG("xQueueCreate heap is full\r\n");
			return -1;
		}
		ret = common_dev_register(&pg_mydev[i]->dev);//add dev to linux kernel
		if(ret == -1){
			LOG("dev %d add err\r\n", i);
			return -1;
		}
		pg_mydev[i]->mutex =  xSemaphoreCreateMutex();//互斥锁
		if(pg_mydev[i]->mutex == pdFALSE){
			LOG("xSemaphoreCreateMutex %d err\r\n", i);
			return -1;
		}
		leinuo_timer_init(pg_mydev[i]);//定时器初始化
	}
	//给不同的雷诺设备命名和设置地址
	pg_mydev[0]->dev.id = Message_Addr_Wifi_LEINUO1;//Message_Addr_Wifi_LEINUO1设备的地址

	memcpy(pg_mydev[0]->dev.name, "LeiNuoWifi1", strlen((char*)"LeiNuoWifi1"));
	
	pg_mydev[1]->dev.id = Message_Addr_Wifi_LEINUO2;//Message_Addr_Wifi_LEINUO2设备的地址
	memcpy(pg_mydev[1]->dev.name, "LeiNuoWifi2", strlen((char*)"LeiNuoWifi2"));
	
	pg_mydev[2]->dev.id = Message_Addr_Wifi_LEINUO3;//Message_Addr_Wifi_LEINUO3设备的地址
	memcpy(pg_mydev[2]->dev.name, "LeiNuoWifi3", strlen((char*)"LeiNuoWifi3"));
}


