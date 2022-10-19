#include "message_handle.h"
#include "usart.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include "mcu_task.h"
#include "wifi_task.h"
#include "gui_task.h"
#include "common_task.h"

extern DMA_HandleTypeDef hdma_usart1_rx;

typedef struct Message_task{
	Common_Task common_task;
	QueueHandle_t Message_Queue;
	uint8_t uart1_Frame_buf[FRAME_MAX_LEN];
	Message_protocol_controller message_protocol_controller;
}Message_task;

static Message_task g_message_task;

//消息协议管理函数
static void* message_protocol_create(uint16_t size);

static uint8_t message_protocol_del(Message_protocol* protocol);

static int8_t message_protocol_register(Message_protocol* protocol);

static Message_protocol* message_find_protocol_by_name(uint8_t* protocol_name);

static int8_t message_protocol_find_type(Frame_t* fram);
//end

//消息相关接口
static char message_check(const Message_t* const mes);
static void message_make_fromISR(Message_t* const frame, const uint8_t* const buf);
//end

//消息协议管理函数
static void* message_protocol_create(uint16_t size){
	return pvPortMalloc(size);
}

static int8_t message_protocol_register(Message_protocol* protocol){
	for(uint16_t i = 0; i < Message_Protocol_Max_Num; i++){
		if(g_message_task.message_protocol_controller.protocols[i] == 0){
			g_message_task.message_protocol_controller.protocols[i] = protocol;
			g_message_task.message_protocol_controller.num++;
			LOG("message_protocol_register success\r\n");
			return pdTRUE;
		}
	}
	LOG("protocol_register buf full\r\n");
	return pdFALSE;
}

static uint8_t common_dev_unregister(Message_protocol* protocol){
	for(uint16_t i = 0; i < Message_Protocol_Max_Num; i++){
		if(protocol == g_message_task.message_protocol_controller.protocols[i]){
			g_message_task.message_protocol_controller.protocols[i] = 0;
		}
	}
}

static uint8_t message_protocol_del(Message_protocol* protocol){
	vPortFree(protocol);
}

static Message_protocol* message_find_protocol_by_name(uint8_t* protocol_name){
	for(uint16_t i = 0; i < Message_Protocol_Max_Num; i++){
		if(memcmp(protocol_name, g_message_task.message_protocol_controller.protocols[i]->name, strlen((char*)protocol_name))){
			return g_message_task.message_protocol_controller.protocols[i];
		}
	}
	LOG("protocol_find_dev_by_name\r\n");
	return NULL;
}

int8_t message_protocol_find_type(Frame_t* fram){
	for(uint8_t i = 0; i < fram->len; i++){
		for(uint8_t j = 0; j < Message_Protocol_Max_Num; j++){
			if(fram->r_buf[i] == g_message_task.message_protocol_controller.protocols[j]->head1){//message head1 ok
				//message head2 ok  
				if(fram->r_buf[i+1] == g_message_task.message_protocol_controller.protocols[j]->head2){
					return g_message_task.message_protocol_controller.protocols[j]->type;
				}
				//message end ok
				if(fram->r_buf[fram->r_buf[i + g_message_task.message_protocol_controller.protocols[j]->len_index] + g_message_task.message_protocol_controller.protocols[j]->len_index_more] == g_message_task.message_protocol_controller.protocols[j]->end){
					return g_message_task.message_protocol_controller.protocols[j]->type;
				}
			}
		}
	}
	return -1;
}

static int8_t message_protocol_find_addr(Frame_t* fram){
	//Determines the length of the received message
//	assert(frame->len <= FRAME_MAX_LEN);
	for(uint8_t i = 0; i < fram->len; i++){
		for(uint8_t j = 0; j < Message_Protocol_Max_Num; j++){
			if(fram->r_buf[i] == g_message_task.message_protocol_controller.protocols[j]->head1){//message head1 ok
				LOG("i = %d, head1 == %.2x\r\n", i, fram->r_buf[i]);
				//message head2 ok  
				if(fram->r_buf[i+1] == g_message_task.message_protocol_controller.protocols[j]->head2){
					LOG("i = %d, head2 == %.2x\r\n", i, fram->r_buf[i + 1]);
					if(g_message_task.message_protocol_controller.protocols[j]->dest_addr_index == 0){
						return -1;
					}
					fram->index_useful = i;
					fram->len = fram->r_buf[i + g_message_task.message_protocol_controller.protocols[j]->len_index] + g_message_task.message_protocol_controller.protocols[j]->len_index_more;
					return fram->r_buf[i + g_message_task.message_protocol_controller.protocols[j]->dest_addr_index];
				}
				//message end ok
				if(fram->r_buf[fram->r_buf[i + g_message_task.message_protocol_controller.protocols[j]->len_index] + g_message_task.message_protocol_controller.protocols[j]->len_index_more] == g_message_task.message_protocol_controller.protocols[j]->end){
					if(g_message_task.message_protocol_controller.protocols[j]->dest_addr_index == 0){
						return -1;
					}
					fram->index_useful = i;
					fram->len = fram->r_buf[i + g_message_task.message_protocol_controller.protocols[j]->len_index] + g_message_task.message_protocol_controller.protocols[j]->len_index_more;
					return fram->r_buf[i + g_message_task.message_protocol_controller.protocols[j]->dest_addr_index];
				}
			}
		}
	}
	return -1;
}

uint8_t* message_protocol_find_name(uint8_t* buf, uint8_t len, uint8_t* index){
	for(uint8_t i = 0; i < len; i++){
		for(uint8_t j = 0; j < Message_Protocol_Max_Num; j++){
			if(buf[i] == g_message_task.message_protocol_controller.protocols[j]->head1){//message head1 ok
				//message head2 ok  
				if(buf[i+1] == g_message_task.message_protocol_controller.protocols[j]->head2){
					*index = i;
					return g_message_task.message_protocol_controller.protocols[j]->name;
				}
				//message end ok
				if(buf[buf[i + g_message_task.message_protocol_controller.protocols[j]->len_index] + g_message_task.message_protocol_controller.protocols[j]->len_index_more] == g_message_task.message_protocol_controller.protocols[j]->end){
					*index = i;
					return g_message_task.message_protocol_controller.protocols[j]->name;
				}
			}
		}
	}
	return NULL;
}

static void message_protocol_copy(Message_protocol* protocol_src, Message_protocol protocol_dest){
	memcpy(protocol_src->name, protocol_dest.name, Message_Protocol_Name_Len);
	protocol_src->type = protocol_dest.type;
	protocol_src->dest_addr_index = protocol_dest.dest_addr_index;
	protocol_src->head1 = protocol_dest.head1;
	protocol_src->head2 = protocol_dest.head2;
	protocol_src->len_index = protocol_dest.len_index;
	protocol_src->len_index_more = protocol_dest.len_index_more;
	protocol_src->end = protocol_dest.end;
	
}
//end

void message_init(){
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	//	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
	//	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart1, g_message_task.uart1_Frame_buf, FRAME_MAX_LEN);
	//	HAL_UART_Receive_DMA(&huart2, uart2_mes_buf, R_MAX_LEN);
	//	HAL_UART_Receive_DMA(&huart3, uart3_mes_buf, R_MAX_LEN);
	g_message_task.Message_Queue = xQueueCreate(MES_QUEUE_LEN, sizeof(Frame_t));
	if(g_message_task.Message_Queue == NULL){
		LOG("\r\nxQueueCreate g_message_task.Message_Queue error\r\n");
	}else{
		LOG("\r\nxQueueCreate g_message_task.Message_Queue success\r\n");
	}
}

void uart1_interrupt_handle(){
	Frame_t frame1_tmp;
	if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) == SET){
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);
		HAL_UART_DMAStop(&huart1);
		frame1_tmp.len = FRAME_MAX_LEN - hdma_usart1_rx.Instance->NDTR;
		memcpy((&frame1_tmp.r_buf[0]), g_message_task.uart1_Frame_buf, frame1_tmp.len);
		//将接收到的消息添加到消息队列
		if(xQueueSendFromISR(g_message_task.Message_Queue, &frame1_tmp, NULL) != pdTRUE){
				LOG("\r\nxQueueSendFromISR error\r\n");
		}
		//清空g_message_task.uart1_Frame_buf
		memset(g_message_task.uart1_Frame_buf, 0, FRAME_MAX_LEN);
		memset(&frame1_tmp, 0, sizeof(Frame_t));
		HAL_UART_Receive_DMA(&huart1, g_message_task.uart1_Frame_buf, FRAME_MAX_LEN);
	}
}

static char message_check(const Message_t* const mes){
	return 1;
}
static void message_make_fromISR(Message_t* const frame, const uint8_t* const buf){
	frame->head1 = buf[0];
	frame->head2 = buf[1];
	frame->addr_src = buf[2];
	frame->addr_dest = buf[3];
	frame->type = buf[4];
	frame->cmd = buf[5];
	frame->len = buf[6];
	memcpy(frame->payload, &buf[7], frame->len);
	frame->check_num = buf[frame->len + 7];
}

static uint8_t message_get_checknum(uint8_t* start, uint8_t len){
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
		message_log("gui mes send", mes->cmd, mes->payload, mes->len);
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
		message_log("mcu mes send", mes->cmd, mes->payload, mes->len);
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
		message_log("leinuo mes send", mes->cmd, mes->payload, mes->len);
	}
	frame.index_useful = 0;
	if(xQueueSend(dev_dest->Message_Queue, &frame, 0) != pdPASS){
		LOG("xQueueSend(gui_get_gui_Queue(), &message_tmp, 0 error\r\n");
	}
}

void message_make_msg_by_protocoltype(uint8_t message_type, uint8_t* buf){

}
void message_handle(const void* const handle){
	message_init();
//	LOG("err = %s\r\n", strerror(errno)); 
	//创建并添加雷诺消息模型
	Message_protocol* message_protocol_leinuo = message_protocol_create(sizeof(Message_protocol));
	//下面为一个协议支持两个类型的设备
	message_protocol_copy(message_protocol_leinuo, 
	(Message_protocol){
		.name = "leinuo", 
		.type = Protocol_Type_G_Gui , 
		.head1=0xaa, 
		.head2=0x33, 
		.len_index = 6, 
		.len_index_more = 8, 
		.end = 0x66
	});
	message_protocol_register(message_protocol_leinuo);
	
	//创建并添加yy消息模型
	Message_protocol* message_protocol_yy = message_protocol_create(sizeof(Message_protocol));
	message_protocol_copy(message_protocol_yy, 
	(Message_protocol){
		.name = "yy", 
		.type = Protocol_Type_Wifi, 
		.head1=0xaa, 
		.head2=0x55, 
		.len_index = 6, 
		.len_index_more = 8, 
		.end = 0x55
	});
	message_protocol_register(message_protocol_yy);
	
	//这是自己mcu内部用的协议
	Message_protocol* message_protocol_mcu = message_protocol_create(sizeof(Message_protocol));
	message_protocol_copy(message_protocol_mcu, 
	(Message_protocol){
		.name = "mcu", 
		.type = MESSAGE_TYPE_MCU, 
		.head1=0xff, 
		.head2=0x55, 
		.len_index = 6, 
		.len_index_more = 8, 
		.dest_addr_index = 3,
	});
	message_protocol_register(message_protocol_mcu);
	
	//这是mygui用的协议
	Message_protocol* message_protocol_mygui = message_protocol_create(sizeof(Message_protocol));
	message_protocol_copy(message_protocol_mygui, 
	(Message_protocol){
		.name = "mygui", 
		.type = MESSAGE_TYPE_MCU, 
		.head1=0xbb, 
		.head2=0x44, 
		.len_index = 6, 
		.len_index_more = 8, 
		.dest_addr_index = 3,
	});
	message_protocol_register(message_protocol_mygui);
	
	
	//这是mygui用的协议
	Message_protocol* message_protocol_kkk = message_protocol_create(sizeof(Message_protocol));
	message_protocol_copy(message_protocol_kkk, 
	(Message_protocol){
		.name = "kkk", 
		.type = MESSAGE_TYPE_MCU, 
		.head1=0x33, 
		.head2=0x66, 
		.len_index = 4, 
		.len_index_more = 6, 
		.dest_addr_index = 0,
	});
	message_protocol_register(message_protocol_kkk);
	Frame_t frame_temp = {};
	int8_t ret = 0;
	uint8_t i = 0;
	int16_t dest_addr = 0;
	uint8_t message_type = 0;
	Dev* dev = 0;
	uint8_t index_useful = 0;
	uint8_t temp_data[FRAME_MAX_LEN] = {};
	dev_controller* dev_con = NULL;
	for(;;){
		ret = xQueueReceive(g_message_task.Message_Queue, &frame_temp ,portMAX_DELAY);
		if(ret == pdPASS){
			//解析出各个消息的目标地址,根据目标地址发送到设备的消息队列中,如果返回值为-1,那么代表着这个类型的消息不支持addr_dest
			dest_addr = message_protocol_find_addr(&frame_temp);
			if(dest_addr == -1){
//				LOG("message_protocol_find_addr = -1\r\n");
				//由于消息并没有addr_dest，所以将此消息广播给所有设备
				dev_con = common_dev_get_controller();
				for(uint8_t i = 0; i < DEV_MAX_NUM; i++){
					if(dev_con->dev[i] != NULL){
						if(dev_con->dev[i]->Message_Queue != NULL){
							ret = xQueueSend(dev_con->dev[i]->Message_Queue, &frame_temp, 0);
							if(ret != pdPASS){
								LOG("dev->Message_Queue full\r\n");
							}
						}
					}
				}
			}else{
				//将消息发送给指定设备的消息队列中等待处理
				dev = common_dev_find_dev_by_addr(dest_addr);
				if(dev != NULL){
					if(dev->Message_Queue != NULL){
						ret = xQueueSend(dev->Message_Queue, &frame_temp, 0);
						if(ret != pdPASS){
							LOG("dev->Message_Queue full\r\n");
						}
					}
				}else{
					LOG("common_dev_find_dev_by_addr error\r\n");
				}
			}
			memset(&frame_temp, 0, sizeof(Frame_t)); 
		}
  }
}

void message_info(const Message_t* const mes){
	printf("\r\n");
	printf("fram.head1 = %.2x\r\n", mes->head1);
	printf("fram.head2 = %.2x\r\n", mes->head2);
	printf("fram.addr_src = %.2x\r\n", mes->addr_src);
	printf("frame->addr_dest = %.2x\r\n", mes->addr_dest);
	printf("frame->type = %.2x\r\n", mes->type);
	printf("frame->cmd = %.2x\r\n", mes->cmd);
	printf("frame->len = %d\r\n", mes->len);
	printf("payload: ");
	for(uint8_t i = 0; i < mes->len; i++){
		printf("%.2x ", mes->payload[i]);
	}
	printf("\r\nframe.check_num = %.2x\r\n", mes->check_num);
}

void message_log(uint8_t* name, uint8_t cmd, uint8_t* payload, uint8_t payload_len){
	LOG("\r\n\r\n\r\n\r\n");
	LOG("------------%s-------------\r\n", name);
	LOG("mes cmd = %.2x\r\n", cmd);
	LOG("mes payload: ");
	for(uint8_t i = 0; i < payload_len; i++){
		LOG("%.2x ", payload[i]);
	}
	LOG("\r\n\r\n\r\n\r\n");
}
void message_send(const Message_t* const mes){
	
}
