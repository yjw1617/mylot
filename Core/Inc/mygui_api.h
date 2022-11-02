#pragma once
#include "DIALOG.h"
#include <stdint.h>
#include "mygui_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gui_task.h"
WM_HWIN mygui_TempCreate(void);

WM_HWIN mygui_AlarmCreate(void);

WM_HWIN mygui_MainCreate(void);

WM_HWIN mygui_LightCreate(void);

WM_HWIN mygui_LedCreate(void);

WM_HWIN mygui_HumCreate(void);

WM_HWIN mygui_FanCreate(void);

/*发送给ui任务的消息函数*/

/*end*/

/*在mygui.c中调用*/
void mygui_set_ui_temp(MyGUI_dev* mydev, uint16_t temp);

void mygui_set_ui_lightness(MyGUI_dev* mydev, uint16_t ligtness);

void mygui_show_ui_temp();

void mygui_set_ui_alarm(MyGUI_dev* mydev, uint8_t flag);

void mygui_set_ui_led(MyGUI_dev* mydev, uint8_t flag);

void mygui_set_ui_fan(MyGUI_dev* mydev, uint8_t flag);

void mygui_show_ui_main();

void mygui_open_lcd(MyGUI_dev* dev);

void mygui_close_lcd(MyGUI_dev* dev);
/*end*/
void mygui_poll_task_handle();



