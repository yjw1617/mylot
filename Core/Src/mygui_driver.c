#include "mygui_driver.h"
#include "message.h"
#include <string.h>
#include "dialog.h"//包含window对话框 头文件
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "utils.h"
//#include "tim.h"
#include "mygui_api.h"
#include "common_event.h"
#define MyGui_Max_NUM 1
#define MYGui_Dev_Queue_Len 2
static MyGUI_dev* pg_mydev[MyGui_Max_NUM];

//falg=0代表num不计入mydev->lcd_lightness
//static void mygui_set_lcd_lightness(MyGUI_dev* mydev, uint8_t falg, uint8_t num){
//	if(num == 0){
//		HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);
//		mydev->lcd.status = MYGUI_LCD_STATUS_OFF;
//		return;
//	}
//	if(falg){
//		mydev->lcd.lightness = num;
//	}	
//	__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, num);
//}

//static void mygui_lcd_wakeup(MyGUI_dev* mydev, uint8_t arg){//arg为0代表没有点击到屏幕的元素，为1代表点击到元素
//	HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
//	A_Log("mydev->lcd.lightness = %d\r\n", mydev->lcd.lightness);
//	__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, mydev->lcd.lightness);
//	if(arg){
//		mydev->lcd.status = MYGUI_LCD_STATUS_ON;
//	}
//	//定时10s变暗
//	mydev->timer[0].change_period(&mydev->timer[0], MYGUI_SHALLOW_SLEEP_DELAY);
//	mydev->timer[0].set_reloadMode(&mydev->timer[0], 0);
//	mydev->timer[0].start(&mydev->timer[0]);
//	//定时15s黑屏
//	mydev->timer[1].change_period(&mydev->timer[1], MYGUI_DEEP_SLEEP_DELAY);
//	mydev->timer[1].set_reloadMode(&mydev->timer[1], 0);
//	mydev->timer[1].start(&mydev->timer[1]);
//}
	


static operations opts = {
//	.msg_parse = msg_parse,
//	.write = write,
};

//static void timer_callback(TimerHandle_t xTimer){
//	//得到mydev的句柄
//	Dev* dev = common_dev_find_dev_by_id(Message_Addr_MY_GUI);
//	MyGUI_dev* mydev = (MyGUI_dev*)(dev->mydev);
//	A_Log("pvTimerGetTimerID = %d\r\n", *((uint32_t*)pvTimerGetTimerID(xTimer)));
//	if(*((uint32_t*)pvTimerGetTimerID(xTimer)) == 0){//10s定时到达
//		A_Log("10s arrive\r\n");
//		mygui_set_lcd_lightness(mydev, 0, MYGUI_SHALLOW_SLLEP_LIGHTNESS);
//	}
//	if(*((uint32_t*)pvTimerGetTimerID(xTimer)) == 1){//15s定时到达
//		A_Log("15s arrive\r\n");
//		mygui_set_lcd_lightness(mydev, 0, 0);
//	}
//	A_Log("gui timer callback\r\n");
//}

//static operations opts = {
////	.msg_parse = msg_parse,
//};

//operations* mygui_get_mydev_p(){
//	return &opts;
//}

int8_t mygui_init(){
	//创建mygui设备
	int8_t ret = -1;
	for(uint8_t i = 0; i < MyGui_Max_NUM; i++){
		pg_mydev[i]= common_mydev_create(sizeof(MyGUI_dev));//创建雷诺结构体
		if(pg_mydev[i] == NULL){
			A_Log("dev_mydev_%d create LeiNuoWifi_dev error\r\n", i);
			return -1;
		}
		memset(pg_mydev[i], 0, sizeof(MyGUI_dev));			//初始化leino结构体为0
		common_dev_opts_init(&pg_mydev[i]->dev, &opts);	//绑定操作函数
		pg_mydev[i]->dev.mydev = pg_mydev[i];						//将自己的mydev指针指向子类
		pg_mydev[i]->dev.Message_Queue = xQueueCreate(MYGui_Dev_Queue_Len , sizeof(Frame_t));
		if(pg_mydev[i]->dev.Message_Queue == NULL){
			A_Log("xQueueCreate heap is full\r\n");
			return -1;
		}
		ret = common_dev_register(&pg_mydev[i]->dev);//add dev to linux kernel
		if(ret == -1){
			A_Log("dev %d add err\r\n", i);
			return -1;
		}
	}
	/*self property*/
	memcpy(pg_mydev[0]->dev.name, "mygui1", strlen((char*)"mygui1"));
	pg_mydev[0]->dev.id = Message_Addr_MY_GUI;
	pg_mydev[0]->lcd.status = MYGUI_LCD_STATUS_OFF;
	pg_mydev[0]->lcd.lightness = 100;
	/*timer*/
	for(uint8_t i = 0 ; i < MYGUI_DEV_TIMRE_NUM; i++){
		pg_mydev[0]->timer[i].autoReload = 0;
		pg_mydev[0]->timer[i].id = i;
		pg_mydev[0]->timer[i].period = 10000;
//		pg_mydev[0]->timer[i].callback_function = timer_callback;
		pg_mydev[0]->timer[i].ticksToWait = 1000;
		commom_timer_init(&pg_mydev[0]->timer[i]);
		pg_mydev[0]->timer[i].create(&pg_mydev[0]->timer[i]);
	}
}

