#include "xiaoyi.h"
#include "message.h"
#include "string.h"

static uint8_t connect(void* my_dev){
	LOG("xiaoyi wifi connect\r\n");
}

static uint8_t on(void* my_dev){
	XiaoyiWifi_dev* mydev = (XiaoyiWifi_dev*)my_dev;
	xSemaphoreTake(mydev->mutex, portMAX_DELAY);
	LOG("xiaoyi wifi %s on\r\n", mydev->dev.name);
	xSemaphoreGive(mydev->mutex);
}

static uint8_t off(void* my_dev){
	XiaoyiWifi_dev* mydev = (XiaoyiWifi_dev*)my_dev;
	xSemaphoreTake(mydev->mutex, portMAX_DELAY);
	LOG("xiaoyi wifi %s off\r\n", mydev->dev.name);
	xSemaphoreGive(mydev->mutex);
}

static uint8_t reset(void* my_dev){
	XiaoyiWifi_dev* mydev = (XiaoyiWifi_dev*)my_dev;
	xSemaphoreTake(mydev->mutex, portMAX_DELAY);
	LOG("xiaoyi wifi reset\r\n");
	xSemaphoreGive(mydev->mutex);
}

static uint8_t ioctl(void* my_dev, uint16_t cmd, uint32_t arg){
	XiaoyiWifi_dev* mydev = (XiaoyiWifi_dev*)my_dev;
	xSemaphoreTake(mydev->mutex, portMAX_DELAY);
	LOG("xiaoyi wifi ioctl\r\n");
	xSemaphoreGive(mydev->mutex);
}

static void timer_callback(TimerHandle_t xTimer){
	LOG("xiaoyi timer callback\r\n");
}

static operations wifi_opts = {
	.on = on,
	.off = off,
	.reset = reset,
	.connect = connect,
	.ioctl = ioctl,
};

operations* xiaoyi_get_mydev_p(){
	return &wifi_opts;
}

void xiaoyi_init(XiaoyiWifi_dev* const mydev, uint16_t dev_addr, const uint8_t* const dev_name, const uint8_t* const timer_name){

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