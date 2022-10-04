#include "utils.h"
#include <stdio.h>
void utils_uint32_to_str(uint8_t* str, uint32_t arg){
	sprintf((char*)str, "%d", arg);
}