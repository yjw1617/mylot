#ifndef _WIFI_H_
#define _WIFI_H_
#include "message.h"
void WifiHandle(void const * argument);
uint8_t Wifi_queue(Message_t *mes, uint32_t delay_ms);
void wifi_init();

#endif