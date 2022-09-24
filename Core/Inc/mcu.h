#ifndef _MCU_H_
#define _MCU_H_
#include <stdint.h>
#include "message.h"
void mcu_mes_deal(const Message_t* const frame);
#endif