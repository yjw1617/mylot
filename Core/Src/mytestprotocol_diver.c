#include "mytestprotocol_driver.h"
#include "message.h"
#include <string.h>
static uint8_t on(void* my_dev){
	A_Log("test wifi on\r\n");
}
static uint8_t off(void* my_dev){
	A_Log("test wifi off\r\n");
}
static uint8_t reset(void* my_dev){
	A_Log("test wifi reset\r\n");
}
static uint8_t connect(void* my_dev){
	A_Log("test wifi connect\r\n");
}
static uint8_t ioctl(void* my_dev, uint16_t cmd, uint32_t arg){
	A_Log("test wifi ioctl\r\n");
}
static operations opts = {
	.on = on,
	.off = off,
	.reset = reset,
	.connect = connect,
};
void mytestwifi_init(TestWifi_dev* const mydev, uint16_t dev_addr, const uint8_t* const dev_name, const uint8_t* const timer_name){
	/*��ʼ���������*/
	mydev->dev.mydev = mydev;
	mydev->dev.id = dev_addr;
	memcpy(mydev->dev.name, dev_name, strlen((char*)dev_name));
	/*��ʼ���ڲ�����*/
	mydev->dev.ops = &opts;
}