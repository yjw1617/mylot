#ifndef _MESSAGE_H_
#define _MESSAGE_H_
#include <stdint.h>
#define SUPPORT_LOG
#ifdef SUPPORT_LOG
#define LOG(...) (printf(__VA_ARGS__))
#else
#define LOG 
#endif
#define PAYLOAD_MAQX_LEN 40
#define FRAME_MAX_LEN (PAYLOAD_MAQX_LEN + 9)
#define MES_TYPE_INDEX 4
#define mes_get_type(frame) (frame[MES_TYPE_INDEX])


#define	MESSAGE_ADDR_MCU   0
#define	MESSAGE_ADDR_WIFI_TEST   1
#define	MESSAGE_ADDR_WIFI_XIAOYI   2
#define MESSAGE_ADDR_MY_GUI 3

#define MESSAGE_CMD_PRINT_MEMORY 0
#define CMD_WIFI_ON 0
#define CMD_WIFI_OFF 1
#define CMD_WIFI_CONNECT 2
#define CMD_WIFI_RESET 3
/*gui界面相关的cmd指令*/
enum CMD_GUI{
	CMD_GUI_UI_SHOW_MAIN = 0,
	CMD_GUI_UI_SHOW_TEMP = 1,
	CMD_GUI_UI_SET_TEMP = 2,
	CMD_GUI_UI_SET_LIGHTNESS = 3,
	CMD_GUI_LCD_OFF = 4,
	CMD_GUI_LCD_WAKEUP = 5,
	CMD_GUI_LCD_SET_LIGHTNESS = 6,
};

#define	MES_TYPE_MCU		0
#define	MES_TYPE_ZIGBEE	1
#define	MES_TYPE_WIFI		2
#define	MESSAGE_TYPE_GUI		3
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

typedef struct Payload_t{
	
}Payload_t;
#define MES_QUEUE_LEN 2
#define MES_QUEUE_ITEM_SIZE sizeof(Message_t)
void uart1_interrupt_handle();
void message_handle(const void* const handle);
void message_init();
void message_info(const Message_t* const message);
void message_send(const Message_t* const mes);
#endif