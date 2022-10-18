#include "uart_dev.h"
#include "message_handle.h"
#include "string.h"
#include "usart.h"
#include "utils.h"
#define Uart_Dev_Max_NUM 3
static Uart_dev* pg_mydev[Uart_Dev_Max_NUM];

static uint8_t on(void* my_dev){
}

static uint8_t off(void* my_dev){
}

static uint8_t reset(void* my_dev){
}

static void timer_callback(TimerHandle_t xTimer){
	LOG("leinuo timer callback\r\n");
}

static int8_t send_mes(Uart_dev* mydev, uint8_t* buf, uint8_t len){
	return HAL_UART_Transmit(mydev->p_uart, buf, len, 0);
}

/*一个设备可以支持多个类型的消息协议*/
static uint8_t msg_parse(void* my_dev, uint8_t* buf, uint8_t len){
	Uart_dev* mydev = (Uart_dev*)my_dev;
	send_mes(mydev, buf, len);
}

static operations opts = {
	.msg_parse = msg_parse,
};

int8_t uart_dev_init(){
	int8_t ret = 0;
	for(uint8_t i = 0; i < Uart_Dev_Max_NUM; i++){
		pg_mydev[i]= common_mydev_create(sizeof(Uart_dev));//创建雷诺结构体
		if(pg_mydev[i] == NULL){
			LOG("dev_mydev_%d create uart_dev error\r\n", i);
			return -1;
		}
		memset(pg_mydev[i], 0, sizeof(Uart_dev));//初始化leino结构体为0
		common_dev_opts_init(&pg_mydev[i]->dev, &opts);//绑定操作函数
		pg_mydev[i]->dev.mydev = pg_mydev[i];//将自己的mydev指针指向子类
		pg_mydev[i]->dev.Message_Queue = xQueueCreate(4 , sizeof(Frame_t));
		if(pg_mydev[i]->dev.Message_Queue == NULL){
			LOG("xQueueCreate heap is full\r\n");
			return -1;
		}
		ret = common_dev_register(&pg_mydev[i]->dev);//add dev to kernel
		if(ret == -1){
			LOG("dev %d add err\r\n", i);
			return -1;
		}
		pg_mydev[i]->mutex =  xSemaphoreCreateMutex();//互斥锁
		if(pg_mydev[i]->mutex == pdFALSE){
			LOG("xSemaphoreCreateMutex %d err\r\n", i);
			return -1;
		}
	}

	pg_mydev[0]->dev.addr = Message_Addr_uart1;//Message_Addr_Wifi_LEINUO1设备的地址
	pg_mydev[0]->p_uart = usart_get_uart(1);
	memcpy(pg_mydev[0]->dev.name, "uart1", strlen((char*)"uart1"));
	
	pg_mydev[1]->dev.addr = Message_Addr_uart2;//Message_Addr_Wifi_LEINUO2设备的地址
	pg_mydev[1]->p_uart = usart_get_uart(2);
	memcpy(pg_mydev[1]->dev.name, "uart2", strlen((char*)"uart2"));
	
	pg_mydev[2]->dev.addr = Message_Addr_uart3;//Message_Addr_Wifi_LEINUO3设备的地址
	pg_mydev[2]->p_uart = usart_get_uart(3);
	memcpy(pg_mydev[2]->dev.name, "uart3", strlen((char*)"uart3"));
}


