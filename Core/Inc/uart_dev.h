#pragma once
#include "message_handle.h"
#include "common_dev.h"
#include "common_timer.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#include "usart.h"
typedef struct Uart_dev{
	UART_HandleTypeDef* p_uart;
	Dev dev;
	SemaphoreHandle_t mutex;
}Uart_dev;
int8_t uart_dev_init();
