#include "leinuo.h"
#include "message.h"
#include "string.h"

static uint8_t connect(void* my_dev){
	LOG("leinuo wifi connect\r\n");
}

static uint8_t on(void* my_dev){
	XiaoyiWifi_dev* mydev = (XiaoyiWifi_dev*)my_dev;
	xSemaphoreTake(mydev->mutex, portMAX_DELAY);
	LOG("leinuo wifi %s on\r\n", mydev->dev.name);
	xSemaphoreGive(mydev->mutex);
}

static uint8_t off(void* my_dev){
	XiaoyiWifi_dev* mydev = (XiaoyiWifi_dev*)my_dev;
	xSemaphoreTake(mydev->mutex, portMAX_DELAY);
	LOG("leinuo wifi %s off\r\n", mydev->dev.name);
	xSemaphoreGive(mydev->mutex);
}

static uint8_t reset(void* my_dev){
	XiaoyiWifi_dev* mydev = (XiaoyiWifi_dev*)my_dev;
	xSemaphoreTake(mydev->mutex, portMAX_DELAY);
	LOG("leinuo wifi reset\r\n");
	xSemaphoreGive(mydev->mutex);
}

static uint8_t ioctl(void* my_dev, uint16_t cmd, uint32_t arg, uint16_t len){
	XiaoyiWifi_dev* mydev = (XiaoyiWifi_dev*)my_dev;
	uint8_t* payload = 0;
	if(len > 0){
		payload = (uint8_t*)arg;
	}
	switch(cmd){
		case CMD_WIFI_XIAOYI_ON:
			on(mydev);
			break;
		case CMD_WIFI_XIAOYI_OFF:
			off(mydev);
			break;
		case CMD_WIFI_XIAOYI_CONNECT:
			connect(mydev);
			break;
		case CMD_WIFI_XIAOYI_RESET:
			reset(mydev);
	}
}

static void timer_callback(TimerHandle_t xTimer){
	LOG("leinuo timer callback\r\n");
}

static operations wifi_opts = {
	.on = on,
	.off = off,
	.reset = reset,
	.connect = connect,
	.ioctl = ioctl,
};

operations* leinuo_get_mydev_p(){
	return &wifi_opts;
}

uint8_t leinuo_whether_leinuo_protocol(uint8_t* frame){
	for(uint8_t i = 0; i < FRAME_MAX_LEN; i++){
		if(frame[i] == XIAOYI_MESSAGE_HEAD1 && frame[i + 1] == XIAOYI_MESSAGE_HEAD2){//如果消息头校验成功
			//校验消息校验位
			
		}
	}
}

void leinuo_init(XiaoyiWifi_dev* const mydev, uint16_t dev_addr, const uint8_t* const dev_name, const uint8_t* const timer_name){

	mydev->dev.mydev = mydev;
	mydev->dev.addr = dev_addr;
	memcpy(mydev->dev.name, dev_name, strlen((char*)dev_name));

	mydev->mutex =  xSemaphoreCreateMutex();
	
	memcpy(mydev->timer.name, timer_name, strlen((char*)timer_name));
	mydev->timer.autoReload = pdTRUE;
	mydev->timer.id = 0;
	mydev->timer.period = 10000;
	mydev->timer.callback_function = timer_callback;
	mydev->timer.ticksToWait = 1000;
	commom_timer_init(&mydev->timer);
	
	mydev->dev.ops = &wifi_opts;
}