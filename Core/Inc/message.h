#ifndef _MESSAGE_H_
#define _MESSAGE_H_
#include <stdint.h>
#define SUPPORT_LOG 1
#if SUPPORT_LOG
#define LOG(...) (printf(__VA_ARGS__))
#endif
#define PAYLOAD_MAQX_LEN 40
#define FRAME_MAX_LEN (PAYLOAD_MAQX_LEN + 9)
#define MES_TYPE_INDEX 4
#define mes_get_type(frame) (frame[MES_TYPE_INDEX])

#define	ADDR_MCU   0
#define	ADDR_ZIGBEE_TEMP   1
#define	ADDR_WIFI   2


enum{
	MES_TYPE_MCU = 0,
	MES_TYPE_ZIGBEE,
	MES_TYPE_WIFI,
};
typedef struct Frame_t{
	uint16_t len;
	uint8_t r_buf[FRAME_MAX_LEN + 20];//增加20以防一包数据中有干扰的数据
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

typedef struct Payload_t{
	
}Payload_t;
#define MES_QUEUE_LEN 10
#define MES_QUEUE_ITEM_SIZE sizeof(Message_t)
void uart1_interrupt_handle();
void MessageHandle(void const * argument);
void log_frame(const Message_t* const message);
void Message_send(const Message_t* const mes);
#endif