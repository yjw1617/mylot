#pragma once
#include "message.h"
#include "common_dev.h"
#include "common_timer.h"
#include "FreeRTOS.h"
#include "timers.h"
#define MYGUI_SHALLOW_SLEEP_DELAY 10000 //浅度睡眠延时
#define MYGUI_DEEP_SLEEP_DELAY    15000 //深度睡眠延时
#define MYGUI_SHALLOW_SLLEP_LIGHTNESS 1 //浅度睡眠lcd的亮度


#define MYGUI_LCD_STATUS_OFF 0
#define MYGUI_LCD_STATUS_ON 1
#define MYGUI_LCD_STATUS_DARK 2

#define MYGUI_DEV_TIMRE_NUM 5	//mygui设备拥有的定时器数目

#define MYGUI_TIMER_NAME_LEN 20

/*gui界面相关的cmd指令*/
#define CMD_GUI_UI_SHOW_MAIN 			0x00
#define CMD_GUI_UI_SHOW_TEMP 			0x01
#define CMD_GUI_UI_SET_TEMP 			0x02
#define CMD_GUI_UI_SET_LIGHTNESS 	0x03
#define CMD_GUI_UI_SET_ALARM 			0x04
#define CMD_GUI_UI_SET_LED 				0x05
#define CMD_GUI_UI_SET_FAN 				0x06
#define CMD_GUI_LCD_OFF 					0x07
#define CMD_GUI_LCD_WAKEUP 				0x08
#define CMD_GUI_LCD_SET_LIGHTNESS 0x09

	
typedef struct MyGUI_dev{
	struct lcd{
		uint8_t lightness;
		uint8_t status;
	}lcd;
	struct ui{
		uint16_t lightness;
		uint8_t temp;
		uint8_t led_status;
		uint8_t fan_status;
		uint8_t alarm_status;
	}ui;
	Dev dev;
	Common_Timer timer[MYGUI_DEV_TIMRE_NUM];
}MyGUI_dev;

int8_t mygui_init();
operations* mygui_get_mydev_p();
