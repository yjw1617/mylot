#include "mygui.h"
#include "message.h"
#include <string.h>
#include "dialog.h"//包含window对话框 头文件
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "utils.h"
#include "tim.h"
#include "mygui_api.h"

//falg=0代表num不计入mydev->lcd_lightness
static void mygui_set_lcd_lightness(MyGUI_dev* mydev, uint8_t falg, uint8_t num){
	if(num == 0){
		HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);
		mydev->lcd_status = MYGUI_LCD_STATUS_OFF;
		return;
	}
	if(num <= 20){//亮度小于=20算暗
		mydev->lcd_status = MYGUI_LCD_STATUS_DARK;
	}
	if(falg){
		mydev->lcd_lightness = num;
	}	
	__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, num);
}

static void mygui_lcd_wakeup(MyGUI_dev* mydev, uint8_t arg){//arg为0代表没有点击到屏幕的元素，为1代表点击到元素
	HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
	__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, mydev->lcd_lightness);
	if(arg){
		mydev->lcd_status = MYGUI_LCD_STATUS_ON;
	}
	//定时10s变暗
	mydev->timer[0].change_period(&mydev->timer[0], MYGUI_SHALLOW_SLEEP_DELAY);
	mydev->timer[0].set_reloadMode(&mydev->timer[0], 0);
	mydev->timer[0].start(&mydev->timer[0]);
	//定时15s黑屏
	mydev->timer[1].change_period(&mydev->timer[1], MYGUI_DEEP_SLEEP_DELAY);
	mydev->timer[1].set_reloadMode(&mydev->timer[1], 0);
	mydev->timer[1].start(&mydev->timer[1]);
}

static uint8_t ioctl(void* my_dev, uint16_t cmd, uint32_t arg, uint16_t len){
	MyGUI_dev* mydev = (MyGUI_dev*)my_dev;
	uint8_t* payload = 0;
	if(len > 0){
		payload = (uint8_t*)arg;
	}
	LOG("my gui ioctl\r\n");
	uint8_t str[10] = {};
	switch(cmd){
		case CMD_GUI_UI_SHOW_MAIN:
			LOG("CMD_GUI_SHOW_MAIN\r\n");
			mygui_show_ui_main();
			break;
		case CMD_GUI_UI_SHOW_TEMP:
			LOG("CMD_GUI_SHOW_TEMP\r\n");
			mygui_show_ui_temp();
			break;
		case CMD_GUI_UI_SET_TEMP:
			utils_uint32_to_str(str, payload[0]);
			LOG("CMD_GUI_SET_TEMP arg=%d  str = %s\r\n ", payload[0], str);
			mydev->ui_temp = arg;
			mygui_set_ui_temp(str);
			break;
		case CMD_GUI_UI_SET_LIGHTNESS:
			utils_uint32_to_str(str, payload[0]);
			LOG("CMD_GUI_UI_SET_LIGHTNESS arg=%d  str = %s\r\n ", payload[0], str);
			mygui_set_ui_lightness(str);
			break;
		case CMD_GUI_LCD_SET_LIGHTNESS:
			utils_uint32_to_str(str, payload[0]);
			LOG("CMD_GUI_LCD_SET_LIGHTNESS arg=%d  str = %s\r\n ", payload[0], str);
			mygui_set_lcd_lightness(mydev, 1, payload[0]);
			break;
		case CMD_GUI_LCD_OFF:
			LOG("CMD_GUI_OFF\r\n");
			//(third num = 0) is stop timer
			mygui_set_lcd_lightness(mydev, 0, 0);
			break;
		case CMD_GUI_LCD_WAKEUP:
			LOG("CMD_GUI_WAKEUP\r\n");
			mygui_lcd_wakeup(mydev, payload[0]);
			break;
		case CMD_GUI_UI_SET_ALARM:
			LOG("CMD_GUI_UI_SET_ALARM\r\n");
			mygui_set_ui_alarm(mydev, payload[0]);
			break;
		case CMD_GUI_UI_SET_LED:
			LOG("CMD_GUI_UI_SET_LED\r\n");
			mygui_set_ui_led(mydev, payload[0]);
			break;
		case CMD_GUI_UI_SET_FAN:
			LOG("CMD_GUI_UI_SET_FAN\r\n");
			mygui_set_ui_fan(mydev, payload[0]);
			break;
	}
}

static void timer_callback(TimerHandle_t xTimer){
	//得到mydev的句柄
	Dev* dev = dev_find_dev_by_addr(MESSAGE_ADDR_MY_GUI);
	MyGUI_dev* mydev = (MyGUI_dev*)(dev->mydev);
	LOG("pvTimerGetTimerID = %d\r\n", *((uint32_t*)pvTimerGetTimerID(xTimer)));
	if(*((uint32_t*)pvTimerGetTimerID(xTimer)) == 0){//10s定时到达
		LOG("10s arrive\r\n");
		mygui_set_lcd_lightness(mydev, 0, MYGUI_SHALLOW_SLLEP_LIGHTNESS);
	}
	if(*((uint32_t*)pvTimerGetTimerID(xTimer)) == 1){//15s定时到达
		LOG("15s arrive\r\n");
		mygui_set_lcd_lightness(mydev, 0, 0);
	}
	LOG("gui timer callback\r\n");
}

static operations mygui_opts = {
	.ioctl = ioctl,
};

operations* mygui_get_mydev_p(){
	return &mygui_opts;
}

void mygui_init(MyGUI_dev* const mydev, uint16_t dev_addr, const uint8_t* const dev_name, const uint8_t* const timer_name){
	mydev->dev.mydev = mydev;
	mydev->dev.addr = dev_addr;
	mydev->lcd_status = MYGUI_LCD_STATUS_OFF;
	mydev->lcd_lightness = 100;
	memcpy(mydev->dev.name, dev_name, strlen((char*)dev_name));
	for(uint8_t i = 0 ; i < MYGUI_DEV_TIMRE_NUM; i++){
		memcpy(mydev->timer[i].name, timer_name, strlen((char*)timer_name));
		mydev->timer[i].autoReload = 0;
		mydev->timer[i].id = i;
		mydev->timer[i].period = 10000;
		mydev->timer[i].callback_function = timer_callback;
		mydev->timer[i].ticksToWait = 1000;
		commom_timer_init(&mydev->timer[i]);
		mydev->timer[i].create(&mydev->timer[i]);
	}
	mydev->dev.ops = &mygui_opts;
}

