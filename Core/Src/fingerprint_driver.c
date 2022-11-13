#include "fingerprint_driver.h"
#include "message.h"
#include "string.h"
#include "usart.h"
#include "utils.h"
#include <stdio.h>
#include "common_event.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "main.h"
#include "utils.h"
#define Dev_uart_status 1		//设备初始uart状态开关
#define Fingerprint_dev_Max_NUM 1
static Fingerprint_dev* pg_mydev[Fingerprint_dev_Max_NUM];

#define Finger_Mes_T(x) 	struct {  										\
														uint8_t head1;							\
														uint8_t head2;							\
														uint8_t mcu_addr1;					\
														uint8_t mcu_addr2;					\
														uint8_t mcu_addr3;					\
														uint8_t mcu_addr4;					\
														uint8_t package_id;					\
														uint8_t len_h;							\
														uint8_t len_l;							\
														uint8_t cmd;								\
														uint8_t data[x];						\
														uint8_t check_num_h;				\
														uint8_t check_num_l;				\
												}																\

static uint16_t get_check_sum(uint8_t* head, uint16_t len){
	uint16_t sum = 0;
	for(uint16_t i = 0; i < len; i++){
		sum += head[i];
	}
	return sum;
}								

static int8_t fingerprint_encapsulate_command_mes_and_send(uint8_t cmd, uint8_t* payload, uint16_t payload_len){
	uint8_t pack_mes[payload_len + 12];//除了payload消息其他数据长度为12
	memset(pack_mes, 0, sizeof(payload_len + 12));
	uint8_t mes_head_to_cmd[] = {0xef, 0x01, 0xff, 0xff, 0xff, 0xff, 0x01};
	memcpy(pack_mes, mes_head_to_cmd, sizeof(mes_head_to_cmd));
	pack_mes[7] = ((payload_len + 3) >> 8) & 0xff;//len high
	pack_mes[8] = (payload_len + 3)& 0xff;//len low
	pack_mes[9] = cmd;//cmd
	memcpy(&pack_mes[10], payload, payload_len);//payload
	uint16_t check_sum = get_check_sum(&pack_mes[6], payload_len + 4);
	pack_mes[payload_len + 10] = (check_sum >> 8) & 0xff;
	pack_mes[payload_len + 11] = check_sum & 0xff;
	A_Log("fingerprint send mes:");
	for(uint8_t i = 0 ; i < payload_len + 12; i++){
		N_Log("%.2x ", pack_mes[i]);
	}
	HAL_UART_Transmit(&huart2, pack_mes, payload_len + 12, 2);
}

static int8_t fingerprint_check_msg(uint8_t* buf, uint8_t len){
	uint8_t payload_len = 0;
	uint16_t check_num = 0;
	uint16_t my_check_num = 0;
	for(uint8_t i = 0 ; i < len; i++){
		if(buf[i] == 0xef && buf[i + 1] == 0x01){
			D_Log("i = %d", i);
			//判断校验和
			payload_len = (buf[i + 7]<<8) + buf[i + 8];
			check_num = (buf[i + payload_len + 7] << 8) + buf[i + payload_len + 8];
			//将数据的前面数字加起来
			my_check_num = utils_calculate_check_num_uint16_t(buf, i + 6, payload_len + 1);
			if(my_check_num == check_num){
				return i;
			}else{
				return -2;
			}
			break;
		}
	}
	return -1;
}

static int8_t fingerprint_parse_msg(uint8_t* buf, uint8_t len){
	uint8_t msg_identifier = buf[6];
	uint8_t msg_config_code = buf[9];
	if(msg_identifier == 0x07){
		switch(msg_config_code){
			case 0x00:
				D_Log("Command executed");
				break;
			case 0x01:
				D_Log("Packet receiving error");
				break;
			case 0x02:
				D_Log("There are no fingers on the sensor");
				break;
			case 0x03:
				D_Log("Description Failed to input fingerprint images");
				break;
			case 0x04:
				D_Log("Fingerprint images are too dry and too light to feature");
				break;
			case 0x05:
				D_Log("Fingerprint images are too wet and mushy to feature");
				break;
			case 0x06:
				D_Log("Fingerprint images are too messy to feature");
				break;
			case 0x07:
				D_Log("The fingerprint image is normal, but the feature points are too small (or the area is too small) to produce a feature");
				break;
			case 0x08:
				D_Log("The prints don't match.");
				break;
			case 0x09:
				D_Log("There were no prints");
				break;
			case 0x0a:
				D_Log("Feature merge failure");
				break;
			case 0x0b:
				D_Log("The address number exceeds the range of the fingerprint database. Procedure");
				break;
			default:
				break;
		}
	}
}

/*一个设备可以支持多个类型的消息协议*/
//有几个设备调用几次
static uint8_t fingerprint_uart_msg_recv(void* my_dev, uint8_t* buf, uint8_t len){
	if(my_dev == NULL){
		D_Log("msg_parse my_dev is null\r\n");
		return -1;
	}
	Fingerprint_dev* mydev = (Fingerprint_dev*)my_dev;
	if(strncmp(mydev->dev.name, "fingerprint1", strlen("fingerprint1")) == 0){
		D_Log("%s recv msg:", mydev->dev.name);
		for(uint8_t i = 0; i < len; i++){
			N_Log("%.2x ", buf[i]);
		}
		//解析这包消息
		int ret = fingerprint_check_msg(buf, len);
		if(ret == -1){
			D_Log("fingerprint parse head error");
			return -1;
		}else if(ret == -2){
			D_Log("fingerprint parse check_num error");
			return -1;
		}
		fingerprint_parse_msg(&buf[ret], len - ret);
	}
}

static int8_t fingerprint_connct_timeout(){
	Error_Check(-1, common_del_temp_event_by_eventType_and_eventId(Event_Type_Wifi, Event_Id_Wifi_Connect));
}

static int8_t fingerprint_connct_suc(){
	Error_Check(-1, common_del_temp_event_by_eventType_and_eventId(Event_Type_Wifi, Event_Id_Wifi_Connect));
	Error_Check(-1, common_del_temp_event_by_eventType_and_eventId(Event_Type_Wifi, Event_Id_Wifi_Connect_TimeOut));
}

static int8_t write(void* my_dev, void* data, uint32_t len){
	if(my_dev == NULL){
		D_Log("msg_parse my_dev is null\r\n");
		return -1;
	}
	Fingerprint_dev* mydev = (Fingerprint_dev*)my_dev;
	D_Log("%s write\r\n", mydev->dev.name);
	//解析数据
	Common_Event* event = (Common_Event*)data;
	int8_t ret = -1;
	Common_Payload* payload = (Common_Payload*)event->data;
	uint8_t fp_data[40] = {0};//需要给指纹模块发送的消息
	switch(event->type){
		case Event_Type_Fingerprint:
			switch(event->id){
				case Event_Id_Fingerprint_Test://指纹检测是否被按下开始
					if(memcmp(payload->data, "start", strlen("start")) == 0){
						common_event_post(Event_Type_Fingerprint, Event_Id_Fingerprint_Test, "run", strlen("run"), payload->delay_ticks, payload->reload);
					}
					if(memcmp(event->data, "run", strlen("run")) == 0){
						//真正检测
						if(HAL_GPIO_ReadPin(finger_int_GPIO_Port, finger_int_Pin)){//如果检测到高电位,关闭test事件,并且150ms之后为指纹模块上电,并且进行自动验证指纹
							vTaskDelay(20);
							if(HAL_GPIO_ReadPin(finger_int_GPIO_Port, finger_int_Pin)){//防抖
								D_Log("finger_int_GPIO_Port is 1");
								Error_Check(-1, common_del_temp_event_by_eventType_and_eventId(Event_Type_Fingerprint, Event_Id_Fingerprint_Test));
								common_event_post(Event_Type_Fingerprint, Event_Id_Fingerprint_Test, "touched", strlen("touched"), 0, 0);
							}
						}
					}
					break;
				case Event_Id_Fingerprint_Power://电源管理
					if(memcmp(event->data, "on", strlen("on")) == 0){//上电
						HAL_GPIO_WritePin(finger_vcc_d1_GPIO_Port, finger_vcc_d1_Pin, 1);
						HAL_GPIO_WritePin(finger_vcc_d2_GPIO_Port, finger_vcc_d2_Pin, 1);
					}
					break;
				case Event_Id_Fingerprint_AutoIdentify://自动验证指纹
					fp_data[0] = 0x03;//risk class
					fp_data[1] = 0xff;
					fp_data[2] = 0xff;//find type
					fp_data[3] = 0x00;
					fp_data[4] = 0x01;
					fingerprint_encapsulate_command_mes_and_send(0x32, fp_data, 5);
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
	.uart_msg_recv = fingerprint_uart_msg_recv,//uart消息解析
};

int8_t fingerprint_dev_init(){
	int8_t ret = 0;
	for(uint8_t i = 0; i < Fingerprint_dev_Max_NUM; i++){
		pg_mydev[i]= common_mydev_create(sizeof(Fingerprint_dev));
		if(pg_mydev[i] == NULL){
			D_Log("dev_mydev_%d create Fingerprint_dev error\r\n", i);
			return -1;
		}
		memset(pg_mydev[i], 0, sizeof(Fingerprint_dev));//初始化leino结构体为0
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
	pg_mydev[0]->dev.id = Dev_id_fingerprint1;//绑定id
	memcpy(pg_mydev[0]->dev.name, "fingerprint1", strlen((char*)"fingerprint1"));//绑定名字
	pg_mydev[0]->dev.uart_enable = Dev_uart_status;//使能uart接口
}


