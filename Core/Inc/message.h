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
											while(!log_flag){vTaskDelay(1);}																				\	
											log_flag = 0;																														\
											printf(__VA_ARGS__);	 																									\
											log_flag = 1;																														\
										}while(0)																																	
#else
#define LOG 
#endif
#define PAYLOAD_MAQX_LEN 40
#define FRAME_MAX_LEN 40
#define MES_TYPE_INDEX 4
#define mes_get_type(frame) (frame[MES_TYPE_INDEX])


#define	MESSAGE_ADDR_MCU   0
#define	MESSAGE_ADDR_WIFI_TEST   1
#define	MESSAGE_ADDR_WIFI_XIAOYI   2
#define MESSAGE_ADDR_WIFI_XIAOYI1	 4
#define MESSAGE_ADDR_MY_GUI 3

										
#define CMD_WIFI_XIAOYI_ON 0
#define CMD_WIFI_XIAOYI_OFF 1
#define CMD_WIFI_XIAOYI_CONNECT 2
#define CMD_WIFI_XIAOYI_RESET 3
#define MESSAGE_CMD_PRINT_MEMORY 0

#define	MESSAGE_TYPE_MCU		0
#define	MESSAGE_TYPE_ZIGBEE	1
#define	MESSAGE_TYPE_WIFI		2
#define	MESSAGE_TYPE_GUI		3


#define Message_Protocol_Max_Num 5	//消息类型的最大数目
#define Message_Protocol_Name_Len 10
typedef struct Frame_t{
	uint16_t len;
	uint8_t r_buf[FRAME_MAX_LEN + 10];
}Frame_t;
typedef struct Message_t{
	uint8_t head1;
	uint8_t head2;
	uint8_t addr_src;
	uint8_t addr_dest;
	uint8_t type;
	uint8_t cmd;
	uint16_t len;
	uint8_t payload[PAYLOAD_MAQX_LEN];
	uint8_t check_num;
}Message_t;

typedef struct Message_protocol{
	uint8_t name[Message_Protocol_Name_Len];
	uint8_t head1;
	uint8_t head2;
	uint8_t len_index;
	uint8_t len_index_more;//知道长度了，需要找数据尾还需要一个数，这个数根据具体协议来决定
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
#endif