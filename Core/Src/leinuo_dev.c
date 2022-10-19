#include "leinuo_dev.h"
#include "message_handle.h"
#include "string.h"
#include "usart.h"
#include "utils.h"
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

static void timer_callback(TimerHandle_t xTimer){
	LOG("leinuo timer callback\r\n");
}

static void leinuo_wake_up(void* my_dev){
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	Dev* p_dev = common_dev_find_dev_by_name("uart1");
	Message_Leinuo_t leinuo_mes = {
		.addr_src = Message_Addr_Wifi_LEINUO1,
		.addr_dest = Message_Addr_Wifi_LEINUO1,
		.type = 0x01,
		.cmd = Leinuo_Cmd_Wake,
		.len = 0,
	};
	message_send_to_dev(p_dev, (uint8_t*)&leinuo_mes,Protocol_Type_Leinuo);
}

/*一个设备可以支持多个类型的消息协议*/
static uint8_t msg_parse(void* my_dev, uint8_t* buf, uint8_t len){
	if(my_dev == NULL){
		LOG("msg_parse my_dev is null\r\n");
		return -1;
	}
	
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	
	LOG("msg_parse dev name = %s\r\n", mydev->dev.name);
	uint8_t* protocol_name = NULL;
	protocol_name = message_protocol_find_name(buf, len, 0);
	if(protocol_name == NULL){
		LOG("not found protocol_name\r\n");
		return -1;
	}
	if(!memcmp(protocol_name, "mcu", strlen("mcu"))){//mygui能听懂mygui语言
		uint8_t cmd = buf[5];
		uint8_t* payload = &buf[7];
		uint8_t len = buf[6];
		message_log((uint8_t*)"LeiNuoWifi_dev mes recv", cmd, payload, len);
		switch(cmd){
			case Leinuo_Cmd_Wake:
				leinuo_wake_up(mydev);
				LOG("Leinuo_Cmd_Wake\r\n");
				break;
			case Leinuo_Cmd_Sleep:
				LOG("Leinuo_Cmd_Sleep\r\n");
				break;
			case Leinuo_Cmd_Connect_Net:
				LOG("Leinuo_Cmd_Connect_Net\r\n ");
				break;
			case Leinuo_Cmd_On:
				LOG("Leinuo_Cmd_On\r\n");
				break;
			case Leinuo_Cmd_Off:
				LOG("Leinuo_Cmd_Off\r\n ");
				break;
			case Leinuo_Cmd_Reset:
				LOG("Leinuo_Cmd_Reset\r\n");
				break;
		}
	}
}

static operations opts = {
	.msg_parse = msg_parse,
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
	pg_mydev[0]->dev.addr = Message_Addr_Wifi_LEINUO1;//Message_Addr_Wifi_LEINUO1设备的地址
	memcpy(pg_mydev[0]->dev.name, "LeiNuoWifi1", strlen((char*)"LeiNuoWifi1"));
	
	pg_mydev[1]->dev.addr = Message_Addr_Wifi_LEINUO2;//Message_Addr_Wifi_LEINUO2设备的地址
	memcpy(pg_mydev[1]->dev.name, "LeiNuoWifi2", strlen((char*)"LeiNuoWifi2"));
	
	pg_mydev[2]->dev.addr = Message_Addr_Wifi_LEINUO3;//Message_Addr_Wifi_LEINUO3设备的地址
	memcpy(pg_mydev[2]->dev.name, "LeiNuoWifi3", strlen((char*)"LeiNuoWifi3"));
}


