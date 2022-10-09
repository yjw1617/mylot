#include "leinuo_service.h"
#include "message.h"
#include "common_dev.h"
static int8_t leinuo_parse_msg(uint8_t* rbuf, uint8_t len);
typedef struct Leinuo_Msg{
	uint8_t head1;
	uint8_t head2;
	uint8_t addr_src;
	uint8_t addr_dest;
	uint8_t type;
	uint8_t cmd;
	uint16_t len;
	uint8_t payload[PAYLOAD_MAX_LEN];
	uint8_t check_num;
}Leinuo_Msg;

static int8_t leinuo_check_checkNum(uint8_t* buf, uint8_t len){
	uint16_t sum = 0;
	
//	for(){
//	
//	}
	return 0;
}

static int8_t leinuo_check_message_len(uint8_t len){
	if(len < 9){
		return -1;
	}
}

static void leinuo_make_msg(Leinuo_Msg* p_Leinuo_Msg, uint8_t* buf){
	p_Leinuo_Msg->head1 = buf[0];
	p_Leinuo_Msg->head2 = buf[1];
	p_Leinuo_Msg->addr_src = buf[2];
	p_Leinuo_Msg->addr_dest = buf[3];
	p_Leinuo_Msg->type = buf[4];
	p_Leinuo_Msg->cmd = buf[5];
	p_Leinuo_Msg->len = buf[6];
	memcpy(p_Leinuo_Msg->payload, &buf[7], p_Leinuo_Msg->len);
	p_Leinuo_Msg->check_num = buf[7 + p_Leinuo_Msg->len];
}

//收到的消息都是为有效数据
int8_t leinuo_deal_recv_msg(uint8_t* rbuf, uint8_t len){
	int8_t ret = 0;
	//如果收到消息小于最小消息则返回错误
	ret = leinuo_check_message_len(len);
	if(ret == -1){
		LOG("leinuo_check_message_len error\r\n");
		return -1;
	}
	//验证雷诺的校验码
	ret = leinuo_check_checkNum(rbuf, len);
	if(ret == -1){
		LOG("leinuo_check_checkNum error\r\n");
		return -1;
	}
	//解析雷诺的消息
	leinuo_parse_msg(rbuf, len);
}

//解析消息
static int8_t leinuo_parse_msg(uint8_t* rbuf, uint8_t len){
	Leinuo_Msg leinuo_msg = {};
	//将消息封装成结构体
	leinuo_make_msg(&leinuo_msg, rbuf);
	//根据dest_addr寻找目标设备
	Dev* dev = dev_find_dev_by_addr(leinuo_msg.addr_dest);
	if(dev == NULL){
		LOG("dev_find_dev_by_addr error\r\n");
		return -1;
	}
	switch(leinuo_msg.cmd){
		case Leinuo_Cmd_On:
			dev->ops->on(dev);
			break;
		case Leinuo_Cmd_Off:
			dev->ops->off(dev);
			break;
		case Leinuo_Cmd_Reset:
			dev->ops->reset(dev);
			break;
	}
}
