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
enum CMD_GUI{
	CMD_GUI_UI_SHOW_MAIN = 0,
	CMD_GUI_UI_SHOW_TEMP = 1,
	CMD_GUI_UI_SET_TEMP = 2,
	CMD_GUI_UI_SET_LIGHTNESS = 3,
	CMD_GUI_UI_SET_ALARM = 7,
	CMD_GUI_UI_SET_LED = 8,
	CMD_GUI_UI_SET_FAN = 9,
	
	CMD_GUI_LCD_OFF = 4,
	CMD_GUI_LCD_WAKEUP = 5,
	CMD_GUI_LCD_SET_LIGHTNESS = 6,
};

typedef struct MyGUI_dev{
	uint8_t lcd_lightness;
	uint8_t lcd_status;
	
	uint8_t ui_lightness;
	uint8_t ui_temp;
	uint8_t ui_led_status;
	uint8_t ui_fan_status;
	uint8_t ui_alarm_status;

	Dev dev;
	Common_Timer timer[MYGUI_DEV_TIMRE_NUM];
}MyGUI_dev;

void mygui_init(MyGUI_dev* const mydev, uint16_t dev_addr, const uint8_t* const dev_name, const uint8_t* const timer_name);
operations* mygui_get_mydev_p();
