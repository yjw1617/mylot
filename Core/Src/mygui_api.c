#include "mygui_api.h"
void mygui_mes_to_ui_show_main(){
	Message_t message = {
		.type = MESSAGE_TYPE_GUI,
		.addr_dest = MESSAGE_ADDR_MY_GUI,
		.cmd = CMD_GUI_UI_SHOW_MAIN,
	};
	if(xQueueSend(gui_get_gui_Queue(), &message, 0) != pdPASS){
		LOG("xQueueSend(gui_get_gui_Queue(), &message_tmp, 0 error\r\n");
	}
}

void mygui_mes_to_lcd_wakeup(uint8_t data){
	Message_t message = {
		.type = MESSAGE_TYPE_GUI,
		.addr_dest = MESSAGE_ADDR_MY_GUI,
		.cmd = CMD_GUI_LCD_WAKEUP,
		.payload = {data},
	};
	if(xQueueSend(gui_get_gui_Queue(), &message, 0) != pdPASS){
		LOG("xQueueSend(gui_get_gui_Queue(), &message_tmp, 0 error\r\n");
	}
}