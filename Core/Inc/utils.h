#pragma once
#include <stdint.h>
void utils_uint32_to_str(uint8_t* str, uint32_t arg);
uint8_t utils_calculate_check_num(uint8_t* msg,uint8_t index_start, uint8_t len);