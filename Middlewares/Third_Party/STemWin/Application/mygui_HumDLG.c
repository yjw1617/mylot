/*********************************************************************
*                                                                    *
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                                                                    *
**********************************************************************
*                                                                    *
* C-file generated by:                                               *
*                                                                    *
*        GUI_Builder for emWin version 5.44                          *
*        Compiled Nov 10 2017, 08:53:57                              *
*        (c) 2017 Segger Microcontroller GmbH & Co. KG               *
*                                                                    *
**********************************************************************
*                                                                    *
*        Internet: www.segger.com  Support: support@segger.com       *
*                                                                    *
**********************************************************************
*/

// USER START (Optionally insert additional includes)
// USER END

#include "mygui_api.h"
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_WINDOW_0 (GUI_ID_USER + 0x00)
#define ID_TEXT_0 (GUI_ID_USER + 0x01)
#define ID_IMAGE_0 (GUI_ID_USER + 0x02)
#define ID_BUTTON_0 (GUI_ID_USER + 0x03)
#define ID_TEXT_1 (GUI_ID_USER + 0x04)

#define ID_IMAGE_0_IMAGE_0 0x00

// USER START (Optionally insert additional defines)
// USER END



// USER START (Optionally insert additional static data)
/**home    **/
extern GUI_CONST_STORAGE GUI_BITMAP bmSubHome;

extern GUI_CONST_STORAGE GUI_BITMAP bmSubHum;

/** ֿ **/
extern GUI_CONST_STORAGE GUI_FONT GUI_Fontfont;
// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 480, 800, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "", ID_TEXT_0, 0, 22, 480, 50, 0, 0x64, 0 },
  { IMAGE_CreateIndirect, "", ID_IMAGE_0, 78, 336, 128, 128, 0, 0, 0 },
  { BUTTON_CreateIndirect, "", ID_BUTTON_0, 344, 674, 64, 64, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "", ID_TEXT_1, 330, 374, 80, 32, 0, 0x64, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};


// USER START (Optionally insert additional static code)
// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
//获取mygui的设备句柄
	Dev* dev = common_dev_find_dev_by_addr(Message_Addr_MY_GUI);
	MyGUI_dev* mydev = (MyGUI_dev*)(dev->mydev);
  const void * pData;
  WM_HWIN      hItem;
  U32          FileSize;
  int          NCode;
  int          Id;
  // USER START (Optionally insert additional variables)
  // USER END

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Window'
    //
    hItem = pMsg->hWin;
    WINDOW_SetBkColor(hItem, GUI_MAKE_COLOR(0x004F2102));
    //
    // Initialization of ''
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
//    TEXT_SetText(hItem, "湿度传感器");
    TEXT_SetFont(hItem, &GUI_Fontfont);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x00FFFFFF));
    //
    // Initialization of ''
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_0);
    IMAGE_SetBitmap(hItem, &bmSubHum);
    //
    // Initialization of ''
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
    TEXT_SetFont(hItem, GUI_FONT_32_ASCII);
    TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x00FFFFFF));
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetText(hItem, "56%");
    // USER START (Optionally insert additional code for further widget initialization)
    /**  ͼƬ    ť  **/
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
    BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmSubHome);
    // USER END
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
		if((mydev->lcd.status != MYGUI_LCD_STATUS_ON) && (NCode == WM_NOTIFICATION_RELEASED)){//如果屏幕不是正常亮的，那将屏幕唤醒
			//发送消息给mugui设备将屏幕唤醒
			//发送消息给mugui设备将屏幕唤醒
			Message_g_gui_t mes = {
				.addr_src = MESSAGE_Addr_MCU,
				.addr_dest = Message_Addr_MY_GUI,
				.cmd = CMD_GUI_LCD_WAKEUP,
				.len = 1,
				.payload = {1},
			};
			message_send_to_dev(&mydev->dev, (uint8_t*)&mes, Protocol_Type_G_Gui);
			return;
		}
		if((mydev->lcd.status == MYGUI_LCD_STATUS_ON) && (NCode == WM_NOTIFICATION_RELEASED)){//如果屏幕是正常亮的，那将屏幕唤醒
			//发送消息给mugui设备将屏幕唤醒
			//发送消息给mugui设备将屏幕唤醒
			Message_g_gui_t mes = {
				.addr_src = MESSAGE_Addr_MCU,
				.addr_dest = Message_Addr_MY_GUI,
				.cmd = CMD_GUI_LCD_WAKEUP,
				.len = 1,
				.payload = {1},
			};
			message_send_to_dev(&mydev->dev, (uint8_t*)&mes, Protocol_Type_G_Gui);
		}
    switch(Id) {
    case ID_BUTTON_0: // Notifications sent by ''
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        GUI_EndDialog(pMsg->hWin, 0);//     Ի   
        mygui_MainCreate();// 򿪾    Ի   
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    // USER START (Optionally insert additional code for further Ids)
    // USER END
    }
    break;
  // USER START (Optionally insert additional message handling)
  // USER END
  default:
    WM_DefaultProc(pMsg);
		//发送消息给mugui设备将屏幕唤醒
			Message_g_gui_t mes = {
				.addr_src = MESSAGE_Addr_MCU,
				.addr_dest = Message_Addr_MY_GUI,
				.cmd = CMD_GUI_LCD_WAKEUP,
				.len = 1,
				.payload = {0},
			};
			message_send_to_dev(&mydev->dev, (uint8_t*)&mes, Protocol_Type_G_Gui);
    break;
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       CreateWindow
*/
WM_HWIN mygui_HumCreate(void);
WM_HWIN mygui_HumCreate(void) {
  WM_HWIN hWin;
  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  return hWin;
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/
