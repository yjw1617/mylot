#pragma once
#include "message.h"
#include "common_dev.h"
#include "common_timer.h"
#include "FreeRTOS.h"
#include "timers.h"
#define MYGUI_LCD_STATUS_OFF 0
#define MYGUI_LCD_STATUS_ON 1
#define MYGUI_LCD_STATUS_DARK 2
#define MYGUI_DEV_TIMRE_NUM 5

#define MYGUI_TIMER_NAME_LEN 20
typedef struct MyGUI_dev{
	uint8_t lcd_lightness;
	uint8_t lcd_status;
	
	uint8_t ui_lightness;
	uint8_t ui_temp;
	uint8_t ui_led_status;
	Dev dev;
	Common_Timer timer[MYGUI_DEV_TIMRE_NUM];
}MyGUI_dev;

void mygui_init(MyGUI_dev* const mydev, uint16_t dev_addr, const uint8_t* const dev_name, const uint8_t* const timer_name);
operations* mygui_get_mydev_p();
