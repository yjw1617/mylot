#include "dev_handle.h"
#include "common_dev.h"
#include "leinuo_dev.h"
//循环遍历各种dev设备的消息队列处理相关事件
void dev_handle(){
	leinuo_dev_init();//初始化leinuo驱动层
	dev_poll_handle();
}