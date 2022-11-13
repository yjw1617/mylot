#include "leinuo_driver.h"
#include "message.h"
#include "string.h"
#include "usart.h"
#include "utils.h"
#include <stdio.h>
#include "common_event.h"
#include "FreeRTOS.h"
#include "semphr.h"

//typedef struct {}
#define LeiNuoWifi_Dev_Max_NUM 3
#define Dev_uart_status        0		//设备初始uart状态开关
static LeiNuoWifi_dev* pg_mydev[LeiNuoWifi_Dev_Max_NUM];
static int8_t leinuo_connct_timeout();
static int8_t leinuo_connct_suc();

/*一个设备可以支持多个类型的消息协议*/
//有几个设备调用几次
static uint8_t leinuo_uart_msg_recv(void* my_dev, uint8_t* buf, uint8_t len){
	if(my_dev == NULL){
		D_Log("msg_parse my_dev is null\r\n");
		return -1;
	}
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	if(strncmp(mydev->dev.name, "LeiNuoWifi1", strlen("LeiNuoWifi1")) == 0){
		D_Log("recv msg\r\n");
		D_Log("read data = %s\r\n", buf);
		if(strncmp(buf, "leinuo1 connect suc", len) == 0){
			//将暂存事件删除
			Error_Check(-1, common_del_temp_event_by_eventType_and_eventId(Event_Type_Wifi, Event_Id_Wifi_Connect));
			Error_Check(-1, common_del_temp_event_by_eventType_and_eventId(Event_Type_Wifi, Event_Id_Wifi_Connect_TimeOut));
			//向应用层上报连接成功
			Error_Check(-1, common_event_post(Event_Type_Wifi, Event_Id_Wifi_Connect_Suc, NULL, NULL, 0, 0));
		}
	}
}

static int8_t leinuo_connct(){
	D_Log("leinuo1 connect.....\r\n");
}

static int8_t leinuo_connct_timeout(){
	Error_Check(-1, common_del_temp_event_by_eventType_and_eventId(Event_Type_Wifi, Event_Id_Wifi_Connect));
}

static int8_t leinuo_connct_suc(){
	Error_Check(-1, common_del_temp_event_by_eventType_and_eventId(Event_Type_Wifi, Event_Id_Wifi_Connect));
	Error_Check(-1, common_del_temp_event_by_eventType_and_eventId(Event_Type_Wifi, Event_Id_Wifi_Connect_TimeOut));
}

static int8_t write(void* my_dev, void* data, uint32_t len){
	if(my_dev == NULL){
		D_Log("msg_parse my_dev is null\r\n");
		return -1;
	}
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	D_Log("%s write\r\n", mydev->dev.name);
	//解析数据
	Common_Event* event = (Common_Event*)data;
	int8_t ret = -1;
	Common_Payload* payload = (Common_Payload*)event->data;
	switch(event->type){
		case Event_Type_Wifi:
			switch(event->id){
				case Event_Id_Wifi_Connect:
					if(memcmp(event->data, "timeout", strlen("timeout")) == 0){
						Error_Check(-1, common_del_temp_event_by_eventType_and_eventId(Event_Type_Wifi, Event_Id_Wifi_Connect));
					}
					if(memcmp(event->data, "on", strlen("on")) == 0){
						Error_Check(-1, leinuo_connct());
					}
					if(memcmp(event->data, "off", strlen("off")) == 0){
	
					}
					break;
				case Event_Id_Wifi_Connect_Redo:
					common_event_post(Event_Type_Wifi, Event_Id_Wifi_Connect, "on", strlen("on"), payload->delay_ticks, payload->reload);
					common_event_post(Event_Type_Wifi, Event_Id_Wifi_Connect_TimeOut, NULL, NULL, payload->timeout, 0);
					break;
				case Event_Id_Wifi_Wakeup:
					break;
				case Event_Id_Wifi_Reset:
					break;
				case Event_Id_Wifi_Connect_TimeOut:
					Error_Check(-1, common_del_temp_event_by_eventType_and_eventId(Event_Type_Wifi, Event_Id_Wifi_Connect));
					break;
			}
			break;
		default:
			D_Log("illegal event\r\n");
			break;
	}
	
}


static operations opts = {
//	.msg_parse = msg_parse,
	.write = write,//app调用的接口
	.uart_msg_recv = leinuo_uart_msg_recv,//uart消息解析
};

int8_t leinuo_dev_init(){
	int8_t ret = 0;
	for(uint8_t i = 0; i < LeiNuoWifi_Dev_Max_NUM; i++){
		pg_mydev[i]= common_mydev_create(sizeof(LeiNuoWifi_dev));//创建雷诺结构体
		if(pg_mydev[i] == NULL){
			D_Log("dev_mydev_%d create LeiNuoWifi_dev error\r\n", i);
			return -1;
		}
		memset(pg_mydev[i], 0, sizeof(LeiNuoWifi_dev));//初始化leino结构体为0
		common_dev_opts_init(&pg_mydev[i]->dev, &opts);//绑定操作函数
		pg_mydev[i]->dev.mydev = pg_mydev[i];//将自己的mydev指针指向子类
		pg_mydev[i]->dev.Message_Queue = xQueueCreate(2 , sizeof(Frame_t));
		if(pg_mydev[i]->dev.Message_Queue == NULL){
			D_Log("xQueueCreate heap is full\r\n");
			return -1;
		}
		ret = common_dev_register(&pg_mydev[i]->dev);//add dev to linux kernel
		if(ret == -1){
			D_Log("dev %d add err\r\n", i);
			return -1;
		}
		pg_mydev[i]->mutex =  xSemaphoreCreateMutex();//互斥锁
		if(pg_mydev[i]->mutex == pdFALSE){
			D_Log("xSemaphoreCreateMutex %d err\r\n", i);
			return -1;
		}
	}
	//给不同的雷诺设备命名和设置地址
	pg_mydev[0]->dev.id = Message_Addr_Wifi_LEINUO1;//Message_Addr_Wifi_LEINUO1设备的地址
	memcpy(pg_mydev[0]->dev.name, "LeiNuoWifi1", strlen((char*)"LeiNuoWifi1"));
	pg_mydev[0]->dev.uart_enable = Dev_uart_status;
	
	pg_mydev[1]->dev.id = Message_Addr_Wifi_LEINUO2;//Message_Addr_Wifi_LEINUO2设备的地址
	memcpy(pg_mydev[1]->dev.name, "LeiNuoWifi2", strlen((char*)"LeiNuoWifi2"));
	pg_mydev[1]->dev.uart_enable = Dev_uart_status;
	
	pg_mydev[2]->dev.id = Message_Addr_Wifi_LEINUO3;//Message_Addr_Wifi_LEINUO3设备的地址
	memcpy(pg_mydev[2]->dev.name, "LeiNuoWifi3", strlen((char*)"LeiNuoWifi3"));
	pg_mydev[2]->dev.uart_enable = Dev_uart_status;
}


