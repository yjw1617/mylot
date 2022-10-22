#ifndef _MESSAGE_H_
#define _MESSAGE_H_
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "common_dev.h"
#define SUPPORT_LOG
#ifdef SUPPORT_LOG
static volatile uint8_t log_flag = 1;
#define LOG(...) do{																																					\
					while(!log_flag){vTaskDelay(1);}	\	
					log_flag = 0;						\
					printf(__VA_ARGS__);	 			\
					log_flag = 1;						\
				}while(0)																																	
#else
#define LOG 
#endif
							
#define PAYLOAD_MAX_LEN 40
#define FRAME_MAX_LEN 40
#define MES_TYPE_INDEX 4
#define mes_get_type(frame) (frame[MES_TYPE_INDEX])

//message head1
enum Message_Head1{
	Message_Head1_leinuo = 0xaa,
};

enum Message_Head2{
	Message_Head2_leinuo = 0x33,
};



enum Message_Addr{
	MESSAGE_Addr_MCU = 0x00,
	MESSAGE_Addr_WIFI_TEST = 0x01,
	Message_Addr_Wifi_LEINUO1 = 0x02,
	Message_Addr_Wifi_LEINUO2 = 0x03,
	Message_Addr_Wifi_LEINUO3 = 0x04,
	Message_Addr_Wifi_LEINUO4 = 0x05,
	Message_Addr_Wifi_LEINUO5 = 0x06,
	Message_Addr_MY_GUI = 0x07,
	Message_Addr_uart1 = 0x08,
	Message_Addr_uart2 = 0x09,
	Message_Addr_uart3 = 0x0a,
};


enum Message_Type{
	MESSAGE_TYPE_MCU  = 0x00,
	MESSAGE_TYPE_ZIGBEE	= 0x01,
	MESSAGE_TYPE_WIFI	= 0x02,
	MESSAGE_TYPE_GUI = 0x03,
};

enum Message_Cmd{
	Message_Uart_Send = 0x00,
};
#define Message_Protocol_Max_Num 5	//消息类型的最大数目
#define Message_Protocol_Name_Len 10
#define Message_Protocol_Type_Len 10

typedef struct Frame_t{
	uint8_t index_useful;
	uint8_t len;
	uint8_t r_buf[FRAME_MAX_LEN];
	uint8_t type;
}Frame_t;
typedef struct Message_t{
	uint8_t head1;
	uint8_t head2;
	uint8_t addr_src;
	uint8_t addr_dest;
	uint8_t type;
	uint8_t cmd;
	uint8_t len;
	uint8_t payload[PAYLOAD_MAX_LEN];
	uint8_t check_num;
}Message_t;

enum Message_Protocol_Type{
	Protocol_Type_Wifi = 0x01,
	Protocol_Type_Zigbee = 0x02,
	Protocol_Type_G_Gui = 0x03,
	Protocol_Type_Mcu = 0x04,
	Protocol_Type_Leinuo = 0x05,
	Protocol_Type_Urgency = 0x06,
	Protocol_Type_Standard = 0x07,
};

typedef struct Message_Leinuo_t{
	uint8_t head1;
	uint8_t head2;
	uint8_t addr_src;
	uint8_t addr_dest;
	uint8_t type;
	uint8_t cmd;
	uint8_t len;
	uint8_t payload[PAYLOAD_MAX_LEN];
	uint8_t check_num;
}Message_Leinuo_t;

typedef struct Message_g_gui_t{
	uint8_t head1;
	uint8_t head2;
	uint8_t addr_src;
	uint8_t addr_dest;
	uint8_t type;
	uint8_t cmd;
	uint8_t len;
	uint8_t payload[PAYLOAD_MAX_LEN];
	uint8_t check_num;
}Message_g_gui_t;

typedef struct Message_Mcu_t{
	uint8_t head1;
	uint8_t head2;
	uint8_t addr_src;
	uint8_t addr_dest;
	uint8_t type;
	uint8_t cmd;
	uint8_t len;
	uint8_t payload[PAYLOAD_MAX_LEN];
	uint8_t check_num;
}Message_Mcu_t;

//消息模型，用于创建消息模型
typedef struct Message_protocol{
	uint8_t name[Message_Protocol_Name_Len];
	uint8_t type;
	uint8_t head1;
	uint8_t head2;
	uint8_t len_index;
	uint8_t len_index_more;//知道长度了，需要找数据尾还需要一个数，这个数根据具体协议来决定
	uint8_t dest_addr_index;//目标设备地址下标
	uint8_t src_addr_index;//发送设备地址下标
	uint8_t end;
}Message_protocol;
//消息模型管理者
typedef struct Message_protocol_controller{
	uint16_t num;
	Message_protocol* protocols[Message_Protocol_Max_Num];
}Message_protocol_controller;

#define MES_QUEUE_LEN 2
#define MES_QUEUE_ITEM_SIZE 40


void message_info(const Message_t* const message);
void message_send(const Message_t* const mes);

//消息协议管理函数
uint8_t* message_protocol_find_name(uint8_t* buf, uint8_t len, uint8_t* index);
int8_t message_protocol_find_type(Frame_t* fram);
int8_t message_protocol_find_addr(Frame_t* fram);
void message_send_to_dev(Dev* dev_dest, uint8_t* message, uint8_t protocol_type);
void message_log(uint8_t type, uint8_t* addr_src_name, uint8_t* addr_dest_name, uint8_t* mes_protocol_name, uint8_t cmd, uint8_t* payload, uint8_t payload_len);

uint8_t message_get_checknum(uint8_t* start, uint8_t len);

uint8_t* message_get_name_by_index(uint8_t id);

void message_log_buf(uint8_t* index_start, uint8_t len);

void* message_protocol_create(uint16_t size);

void message_protocol_copy(Message_protocol* protocol_src, Message_protocol protocol_dest);

int8_t message_protocol_register(Message_protocol* protocol);


//end
#endif