#pragma once
#include <stdint.h>
#define Leinuo_Cmd_On 0x00
#define Leinuo_Cmd_Off 0x01
#define Leinuo_Cmd_Reset 0x02
#define Leinuo_Cmd_Connect_Net 0x03

int8_t leinuo_deal_recv_msg(uint8_t* rbuf, uint8_t len);