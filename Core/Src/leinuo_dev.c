#include "leinuo_dev.h"
#include "message.h"
#include "string.h"
#include "usart.h"
#include "utils.h"
#define LeiNuoWifi_Dev_Max_NUM 3
static LeiNuoWifi_dev* pg_mydev[LeiNuoWifi_Dev_Max_NUM];

static void send_message(Leinuo_Msg msg){
	uint8_t w_buf[FRAME_MAX_LEN] = {};
	w_buf[0] = msg.head1;
	w_buf[1] = msg.head2;
	w_buf[2] = msg.addr_src;
	w_buf[3] = msg.addr_dest;
	w_buf[4] = msg.type;
	w_buf[5] = msg.cmd;
	w_buf[6] = msg.len;
	memcpy(&w_buf[7], msg.payload , msg.len);
	//计算出check_num
	w_buf[msg.len + 7] = utils_calculate_check_num(w_buf, 0, 7 + msg.len);
	//发送消息
	HAL_StatusTypeDef ret = HAL_UART_Transmit(&huart1, w_buf, msg.len + 8, 5);
}

static uint8_t connect(void* my_dev){
	LOG("wifi connect\r\n");
}

static uint8_t on(void* my_dev){
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	xSemaphoreTake(mydev->mutex, portMAX_DELAY);
	LOG("wifi %s on\r\n", mydev->dev.name);
	send_message((Leinuo_Msg)
	{
	.head1 = 0xaa, 
	.head2 = 0x33, 
	.addr_src = 0x00, 
	.addr_dest = 0x01, 
	.cmd = 0x33, 
	.type = 0x22, 
	.len = 10,
	.payload = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x10},
	}
	);
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


/*一个设备可以支持多个类型的消息协议*/
static uint8_t msg_parse(void* my_dev, uint8_t* buf, uint8_t len){
	LeiNuoWifi_dev* mydev = (LeiNuoWifi_dev*)my_dev;
	uint8_t data[len];
	memcpy(data, buf, len);
	//将消息解析出来封装成结构体
	Message_Leinuo_t* mes = (Message_Leinuo_t*)data;
	if(mes->addr_src == MESSAGE_Addr_MCU){	//发送消息的是mcu,根据mcu说的语言来解析
		switch(mes->cmd){
			case Leinuo_Cmd_Wake:
					
				break;
			case Leinuo_Cmd_Off:
				
				break;
		}
	}else if(mes->addr_src == Message_Addr_Wifi_LEINUO1){ //发送消息的是Message_Addr_Wifi_LEINUO1,根据Message_Addr_Wifi_LEINUO1说的语言来解析

	}
}

static operations opts = {
	.on = on,
	.off = off,
	.reset = reset,
	.connect = connect,
	.ioctl = ioctl,
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


