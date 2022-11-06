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
#include <stdio.h>
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
extern GUI_CONST_STORAGE GUI_BITMAP bmSubTemp;

extern GUI_CONST_STORAGE GUI_BITMAP bmSubHome;

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
    TEXT_SetText(hItem, "温度");
    TEXT_SetFont(hItem, &GUI_Fontfont);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x00FFFFFF));
    //
    // Initialization of ''
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_0);
    IMAGE_SetBitmap(hItem, &bmSubTemp);//     ¶ ͼ  
    //
    // Initialization of ''
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
    TEXT_SetFont(hItem, GUI_FONT_32_ASCII);
    TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x00FFFFFF));
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetText(hItem, "32");
    // USER START (Optionally insert additional code for further widget initialization)
    /**  ͼƬ    ť  **/
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
    BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmSubHome);
    // USER END
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_BUTTON_0: // Notifications sent by ''
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
				LOG("show main\r\n");
        GUI_EndDialog(pMsg->hWin, 0);
				mygui_MainCreate();
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
static WM_HWIN hWin;
WM_HWIN mygui_TempCreate(void);
WM_HWIN mygui_TempCreate(void) {
  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  return hWin;
}


void mygui_set_ui_temp(MyGUI_dev* mydev, uint16_t temp){
	mydev->ui.temp = temp;
	WM_HWIN hItem;
	hItem = WM_GetDialogItem(hWin, ID_TEXT_1);
	uint8_t str[10] = {};
	//将uint16转为字符串
	sprintf((char*)str ,"%d", temp);
	TEXT_SetText(hItem, (char*)str);
}

void mygui_show_ui_temp(){
	GUI_EndDialog(hWin, 0);
	mygui_TempCreate();
}
// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/
