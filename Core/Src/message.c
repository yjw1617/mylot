#include <stdint.h>
#include "message.h"
#include <string.h>

static Message_protocol_controller message_protocol_controller;

//对应每一个设备的数组
static uint8_t* Message_addr_name_arry[] = {
	"mcu",
	"wifi_test",
	"wifi_leinuo1",
	"wifi_leinuo2",
	"wifi_leinuo3",
	"wifi_leinuo4",
	"wifi_leinuo5",
	"my_gui",
	"uart1",
	"uart2",
	"uart3",
};
//根据id返回设备名
uint8_t* message_get_name_by_index(uint8_t id){
	return Message_addr_name_arry[id];
}

//消息协议管理函数
void* message_protocol_create(uint16_t size){
	return pvPortMalloc(size);
}

int8_t message_protocol_register(Message_protocol* protocol){
	for(uint16_t i = 0; i < Message_Protocol_Max_Num; i++){
		if(message_protocol_controller.protocols[i] == 0){
			message_protocol_controller.protocols[i] = protocol;
			message_protocol_controller.num++;
			A_Log("message_protocol_register success\r\n");
			return pdTRUE;
		}
	}
	A_Log("protocol_register buf full\r\n");
	return pdFALSE;
}

uint8_t message_protocol_unregister(Message_protocol* protocol){
	for(uint16_t i = 0; i < Message_Protocol_Max_Num; i++){
		if(protocol == message_protocol_controller.protocols[i]){
			message_protocol_controller.protocols[i] = 0;
		}
	}
}

uint8_t message_protocol_del(Message_protocol* protocol){
	vPortFree(protocol);
}

Message_protocol* message_find_protocol_by_name(uint8_t* protocol_name){
	for(uint16_t i = 0; i < Message_Protocol_Max_Num; i++){
		if(memcmp(protocol_name, message_protocol_controller.protocols[i]->name, strlen((char*)protocol_name))){
			return message_protocol_controller.protocols[i];
		}
	}
	A_Log("protocol_find_dev_by_name\r\n");
	return NULL;
}

int8_t message_protocol_find_type(Frame_t* fram){
	for(uint8_t i = 0; i < fram->len; i++){
		for(uint8_t j = 0; j < Message_Protocol_Max_Num; j++){
			if(fram->r_buf[i] == message_protocol_controller.protocols[j]->head1){//message head1 ok
				//message head2 ok  
				if(fram->r_buf[i+1] == message_protocol_controller.protocols[j]->head2){
					return message_protocol_controller.protocols[j]->type;
				}
				//message end ok
				if(fram->r_buf[fram->r_buf[i + message_protocol_controller.protocols[j]->len_index] + message_protocol_controller.protocols[j]->len_index_more] == message_protocol_controller.protocols[j]->end){
					return message_protocol_controller.protocols[j]->type;
				}
			}
		}
	}
	return -1;
}

int8_t message_protocol_find_addr(Frame_t* fram){
	for(uint8_t i = 0; i < fram->len; i++){
		for(uint8_t j = 0; j < Message_Protocol_Max_Num; j++){
			if(fram->r_buf[i] == message_protocol_controller.protocols[j]->head1){//message head1 ok
				//message head2 ok  
				if(fram->r_buf[i+1] == message_protocol_controller.protocols[j]->head2){
					if(message_protocol_controller.protocols[j]->dest_addr_index == 0){
						return -1;
					}
					fram->index_useful = i;
					fram->len = fram->r_buf[i + message_protocol_controller.protocols[j]->len_index] + message_protocol_controller.protocols[j]->len_index_more;
					return fram->r_buf[i + message_protocol_controller.protocols[j]->dest_addr_index];
				}
				//message end ok
				if(fram->r_buf[fram->r_buf[i + message_protocol_controller.protocols[j]->len_index] + message_protocol_controller.protocols[j]->len_index_more] == message_protocol_controller.protocols[j]->end){
					if(message_protocol_controller.protocols[j]->dest_addr_index == 0){
						return -1;
					}
					fram->index_useful = i;
					fram->len = fram->r_buf[i + message_protocol_controller.protocols[j]->len_index] + message_protocol_controller.protocols[j]->len_index_more;
					return fram->r_buf[i + message_protocol_controller.protocols[j]->dest_addr_index];
				}
			}
		}
	}
	return -1;
}

uint8_t* message_protocol_find_name(uint8_t* buf, uint8_t len, uint8_t* index){
	for(uint8_t i = 0; i < len; i++){
		for(uint8_t j = 0; j < Message_Protocol_Max_Num; j++){
			if(buf[i] == message_protocol_controller.protocols[j]->head1){//message head1 ok
				//message head2 ok  
				if(buf[i+1] == message_protocol_controller.protocols[j]->head2){
					*index = i;
					return message_protocol_controller.protocols[j]->name;
				}
				//message end ok
				if(buf[buf[i + message_protocol_controller.protocols[j]->len_index] + message_protocol_controller.protocols[j]->len_index_more] == message_protocol_controller.protocols[j]->end){
					*index = i;
					return message_protocol_controller.protocols[j]->name;
				}
			}
		}
	}
	return NULL;
}

void message_protocol_copy(Message_protocol* protocol_src, Message_protocol protocol_dest){
	memcpy(protocol_src->name, protocol_dest.name, Message_Protocol_Name_Len);
	protocol_src->type = protocol_dest.type;
	protocol_src->dest_addr_index = protocol_dest.dest_addr_index;
	protocol_src->head1 = protocol_dest.head1;
	protocol_src->head2 = protocol_dest.head2;
	protocol_src->len_index = protocol_dest.len_index;
	protocol_src->len_index_more = protocol_dest.len_index_more;
	protocol_src->end = protocol_dest.end;
}

static uint8_t* message_find_protocolname_by_protocoltype(uint8_t type){
	for(uint8_t j = 0; j < Message_Protocol_Max_Num; j++){
		if(message_protocol_controller.protocols[j]->type == type){
			return message_protocol_controller.protocols[j]->name;
		}
	}
}
//end





static char message_check(const Message_t* const mes){
	return 1;
}


uint8_t message_get_checknum(uint8_t* start, uint8_t len){
	uint8_t sum = 0;
	for(uint8_t i = 0; i < len; i++){
		sum += start[i];
	}
	return sum % 256;
}

void message_send_to_dev(Dev* dev_dest, uint8_t* message, uint8_t protocol_type){
	Frame_t frame = {};
	if(protocol_type == Protocol_Type_G_Gui){
		Message_g_gui_t* mes = (Message_g_gui_t*)message;
		frame.r_buf[0] = 0xbb,
		frame.r_buf[1] = 0x44,
		frame.r_buf[2] = mes->addr_src,
		frame.r_buf[3] = mes->addr_dest,
//		frame.r_buf[4] = mes->type,
		frame.r_buf[5] = mes->cmd,
		frame.r_buf[6] = mes->len,
		memcpy(&frame.r_buf[7], mes->payload, mes->len);
		frame.r_buf[7 + mes->len] = message_get_checknum(&frame.r_buf[0], mes->len + 8);//检验码是所有数据之和%256
		frame.len = 8 + mes->len;
		message_log(1, message_get_name_by_index(mes->addr_src), message_get_name_by_index(mes->addr_dest), message_find_protocolname_by_protocoltype(protocol_type), mes->cmd, mes->payload, mes->len);
	}
	if(protocol_type == Protocol_Type_Mcu){
		Message_Mcu_t* mes = (Message_Mcu_t*)message;
		//封包
		frame.r_buf[0] = 0xff,
		frame.r_buf[1] = 0x55,
		frame.r_buf[2] = mes->addr_src,
		frame.r_buf[3] = mes->addr_dest,
		frame.r_buf[4] = mes->type,
		frame.r_buf[5] = mes->cmd,
		frame.r_buf[6] = mes->len,
		memcpy(&frame.r_buf[7], mes->payload, mes->len);
		frame.r_buf[mes->len + 7] = message_get_checknum(&frame.r_buf[0], mes->len + 8);
		frame.len = 8 + mes->len;
		message_log(1, message_get_name_by_index(mes->addr_src), message_get_name_by_index(mes->addr_dest), message_find_protocolname_by_protocoltype(protocol_type), mes->cmd, mes->payload, mes->len);
	}
	if(protocol_type == Protocol_Type_Leinuo){
		Message_Leinuo_t* mes = (Message_Leinuo_t*)message;
		//封包
		frame.r_buf[0] = 0xaa,
		frame.r_buf[1] = 0x33,
		frame.r_buf[2] = mes->addr_src,
		frame.r_buf[3] = mes->addr_dest,
		frame.r_buf[4] = mes->type,
		frame.r_buf[5] = mes->cmd,
		frame.r_buf[6] = mes->len,
		memcpy(&frame.r_buf[7], mes->payload, mes->len);
		frame.r_buf[mes->len + 7] = message_get_checknum(&frame.r_buf[0], mes->len + 8);
		frame.len = 8 + mes->len;
		message_log(1, message_get_name_by_index(mes->addr_src), message_get_name_by_index(mes->addr_dest), message_find_protocolname_by_protocoltype(protocol_type), mes->cmd, mes->payload, mes->len);
	}
	frame.index_useful = 0;
	if(xQueueSend(dev_dest->Message_Queue, &frame, 0) != pdPASS){
		A_Log("xQueueSend(gui_get_gui_Queue(), &message_tmp, 0 error\r\n");
	}
}

void message_log(uint8_t type, uint8_t* addr_src_name, uint8_t* addr_dest_name, uint8_t* mes_protocol_name, uint8_t cmd, uint8_t* payload, uint8_t payload_len){
	A_Log("\r\n\r\n\r\n\r\n");
	if(type == 0){
		A_Log("------------%s recv %s %s msg-------------\r\n", addr_src_name, addr_dest_name, mes_protocol_name);
	}else if(type == 1){
		A_Log("------------%s send to %s %s msg-------------\r\n", addr_src_name, addr_dest_name, mes_protocol_name);
	}
	A_Log("mes cmd = %.2x\r\n", cmd);
	A_Log("mes payload: ");
	for(uint8_t i = 0; i < payload_len; i++){
		A_Log("%.2x ", payload[i]);
	}
	A_Log("\r\n\r\n\r\n\r\n");
}

void message_log_buf(uint8_t* index_start, uint8_t len){
	for(uint8_t i = 0 ; i < len; i++){
		A_Log("%.2x ", index_start[i]);
	}
}