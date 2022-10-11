#ifndef _MESSAGE_H_
#define _MESSAGE_H_
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
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
	MESSAGE_Addr_MY_GUI = 0x07,
};

enum Message_Type{
	MESSAGE_TYPE_MCU  = 0x00,
	MESSAGE_TYPE_ZIGBEE	= 0x01,
	MESSAGE_TYPE_WIFI	= 0x02,
	MESSAGE_TYPE_GUI = 0x03,
};

enum Message_Cmd{
	Leinuo_Cmd_On = 0x00,
	Leinuo_Cmd_Off = 0x01,
	Leinuo_Cmd_Reset = 0x02,
	Leinuo_Cmd_Connect_Net = 0x03,
};

#define Message_Protocol_Max_Num 5	//消息类型的最大数目
#define Message_Protocol_Name_Len 10
#define Message_Protocol_Type_Len 10
typedef struct Frame_t{
	uint16_t len;
	uint8_t r_buf[FRAME_MAX_LEN];
}Frame_t;
typedef struct Message_t{
	uint8_t head1;
	uint8_t head2;
	uint8_t addr_src;
	uint8_t addr_dest;
	uint8_t type;
	uint8_t cmd;
	uint16_t len;
	uint8_t payload[PAYLOAD_MAX_LEN];
	uint8_t check_num;
}Message_t;

enum Message_Protocol_Type{
	Protocol_Type_Wifi = (1<<0),
	Protocol_Type_Zigbee = (1<<1),
	Protocol_Type_Gui = (1<<2),
};

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

typedef struct Message_protocol_controller{
	uint16_t num;
	Message_protocol* protocols[Message_Protocol_Max_Num];
}Message_protocol_controller;

typedef struct Payload_t{
	
}Payload_t;
#define MES_QUEUE_LEN 2
#define MES_QUEUE_ITEM_SIZE 40
void uart1_interrupt_handle();
void message_handle(const void* const handle);
void message_init();
void message_info(const Message_t* const message);
void message_send(const Message_t* const mes);

//消息协议管理函数
uint8_t* message_protocol_find_name(uint8_t* buf, uint8_t len, uint8_t* index);
//end
#endif