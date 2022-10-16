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
#define MyGui_Max_NUM 1
#define MYGui_Dev_Queue_Len 2
static MyGUI_dev* pg_mydev[MyGui_Max_NUM];

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
	LOG("dev name=%s\r\n ", mydev->dev.name);
	LOG("mygui_lcd_wakeup start\r\n");
	HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
	
	__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, mydev->lcd_lightness);
	if(arg){
		mydev->lcd_status = MYGUI_LCD_STATUS_ON;
	}
	LOG("11\r\n");
	//定时10s变暗
	mydev->timer[0].change_period(&mydev->timer[0], MYGUI_SHALLOW_SLEEP_DELAY);
	mydev->timer[0].set_reloadMode(&mydev->timer[0], 0);
	mydev->timer[0].start(&mydev->timer[0]);
	LOG("22\r\n");
	//定时15s黑屏
	mydev->timer[1].change_period(&mydev->timer[1], MYGUI_DEEP_SLEEP_DELAY);
	mydev->timer[1].set_reloadMode(&mydev->timer[1], 0);
	mydev->timer[1].start(&mydev->timer[1]);
	LOG("33\r\n");
}

//static void mygui_refresh(MyGUI_dev* mydev,uint32_t delayms){
//	vTaskDelay(delayms);
//	uint8_t str[10] = {0};
//	/*update temp*/
//	memset(str, 0, 10);
//	sprintf((char*)str, "%d", mydev->ui_temp);
//	mygui_set_ui_temp(str);
//	
//	/*update ligtness*/
//	memset(str, 0, 10);
//	sprintf((char*)str, "%d", mydev->ui_lightness);
//	mygui_set_ui_lightness(str);
//	
//	/*update alarm*/
//	memset(str, 0, 10);
//	sprintf((char*)str, "%d", mydev->ui_alarm_status);
//	mygui_set_ui_alarm(str);
//	
//	/*update alarm*/
//	memset(str, 0, 10);
//	sprintf((char*)str, "%d", mydev->ui_alarm_status);
//	mygui_set_ui_alarm(str);
//}
	
static uint8_t msg_parse(void* my_dev, uint8_t* buf, uint8_t len){
	MyGUI_dev* mydev = (MyGUI_dev*)my_dev;
	LOG("msg_parse dev name = %s\r\n", mydev->dev.name);
	uint8_t* protocol_name = NULL;
	protocol_name = message_protocol_find_name(buf, len, 0);
	if(protocol_name == NULL){
		return -1;
	}
	if(memcpy(protocol_name, "mygui", strlen("mygui"))){//mygui能听懂mygui语言
		uint8_t cmd = buf[5];
		uint8_t* payload = &buf[7];
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
				LOG("CMD_GUI_SET_TEMP arg=%d \r\n ", payload[0]);
				mygui_set_ui_temp(mydev, payload[0]);
				break;
			case CMD_GUI_UI_SET_LIGHTNESS:
				LOG("CMD_GUI_UI_SET_LIGHTNESS arg=%d\r\n ", (payload[0] << 8) + payload[1]);
				mygui_set_ui_lightness(mydev, (payload[0] << 8) + payload[1]);
				break;
			case CMD_GUI_LCD_SET_LIGHTNESS:
				LOG("CMD_GUI_LCD_SET_LIGHTNESS arg1=%d \r\n ", payload[0]);
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
	}else if(memcpy(protocol_name, "mcu", strlen("mcu"))){//mygui能听懂mcu语言
		uint8_t cmd = buf[5];
		uint8_t* payload = &buf[7];
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
				LOG("CMD_GUI_SET_TEMP arg=%d \r\n ", payload[0]);
				mygui_set_ui_temp(mydev, payload[0]);
				break;
			case CMD_GUI_UI_SET_LIGHTNESS:
				LOG("CMD_GUI_UI_SET_LIGHTNESS arg=%d\r\n ", (payload[0] << 8) + payload[1]);
				mygui_set_ui_lightness(mydev, (payload[0] << 8) + payload[1]);
				break;
			case CMD_GUI_LCD_SET_LIGHTNESS:
				LOG("CMD_GUI_LCD_SET_LIGHTNESS arg2=%d \r\n ", payload[0]);
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
}

static void timer_callback(TimerHandle_t xTimer){
	//得到mydev的句柄
	Dev* dev = common_dev_find_dev_by_addr(Message_Addr_MY_GUI);
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

static operations opts = {
	.msg_parse = msg_parse,
};

operations* mygui_get_mydev_p(){
	return &opts;
}

int8_t mygui_init(){
	//创建mygui设备
	int8_t ret = -1;
	for(uint8_t i = 0; i < MyGui_Max_NUM; i++){
		pg_mydev[i]= common_mydev_create(sizeof(MyGUI_dev));//创建雷诺结构体
		if(pg_mydev[i] == NULL){
			LOG("dev_mydev_%d create LeiNuoWifi_dev error\r\n", i);
			return -1;
		}
		memset(pg_mydev[i], 0, sizeof(MyGUI_dev));			//初始化leino结构体为0
		common_dev_opts_init(&pg_mydev[i]->dev, &opts);	//绑定操作函数
		pg_mydev[i]->dev.mydev = pg_mydev[i];						//将自己的mydev指针指向子类
		pg_mydev[i]->dev.Message_Queue = xQueueCreate(MYGui_Dev_Queue_Len , sizeof(Frame_t));
		if(pg_mydev[i]->dev.Message_Queue == NULL){
			LOG("xQueueCreate heap is full\r\n");
			return -1;
		}
		ret = common_dev_register(&pg_mydev[i]->dev);//add dev to linux kernel
		if(ret == -1){
			LOG("dev %d add err\r\n", i);
			return -1;
		}
	}
	/*self property*/
	memcpy(pg_mydev[0]->dev.name, "mygui1", strlen((char*)"mygui1"));
	pg_mydev[0]->dev.addr = Message_Addr_MY_GUI;
	pg_mydev[0]->lcd_status = MYGUI_LCD_STATUS_OFF;
	pg_mydev[0]->lcd_lightness = 100;
	/*timer*/
	for(uint8_t i = 0 ; i < MYGUI_DEV_TIMRE_NUM; i++){
		pg_mydev[0]->timer[i].autoReload = 0;
		pg_mydev[0]->timer[i].id = i;
		pg_mydev[0]->timer[i].period = 10000;
		pg_mydev[0]->timer[i].callback_function = timer_callback;
		pg_mydev[0]->timer[i].ticksToWait = 1000;
		commom_timer_init(&pg_mydev[0]->timer[i]);
		pg_mydev[0]->timer[i].create(&pg_mydev[0]->timer[i]);
	}
}

