#include "utils.h"
#include <stdio.h>
#include "message.h"
void utils_uint32_to_str(uint8_t* str, uint32_t arg){
	sprintf((char*)str, "%d", arg);
}

//数据的校验位等于所有数据相加对256取余
//index_start:从数据的index_start位置开始取数
//len:加入校验的有多少位数据
uint8_t utils_calculate_check_num_uint8_t(uint8_t* msg,uint8_t index_start, uint8_t len){
	uint8_t sum = 0;
	for(uint8_t i = index_start; i < len + index_start; i++){
		sum += msg[i];
	}
	return sum % 256;
}

//数据的校验位等于所有数据相加对2^16-1取余
//index_start:从数据的index_start位置开始取数
//len:加入校验的有多少位数据
uint16_t utils_calculate_check_num_uint16_t(uint8_t* msg,uint8_t index_start, uint8_t len){
	uint16_t sum = 0;
	for(uint8_t i = index_start; i < len + index_start; i++){
		sum += msg[i];
	}
	return sum % 65535;
}