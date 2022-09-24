#include "wifi.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
static QueueHandle_t Wifi_Queue;
void wifi_mes_deal(Message_t* frame){
	log_frame(frame);
}
 
uint8_t Wifi_queue(Message_t *mes, uint32_t delay_ms){
	return xQueueSend(Wifi_Queue, &mes, pdMS_TO_TICKS(delay_ms));
}

uint8_t Wifi_dequeue(Message_t *mes, uint32_t delay_ms){
	return xQueueReceive( Wifi_Queue, &mes, pdMS_TO_TICKS(delay_ms));
}

void wifi_init(){
	Wifi_Queue = xQueueCreate(MES_QUEUE_LEN, MES_QUEUE_ITEM_SIZE);
	if(Wifi_Queue == NULL){
		
	}
}

void WifiHandle(void const * argument){
	Message_t message_tmp = {0};
	wifi_init();
	//处理wifi消息队列
	for(;;){
		uint8_t ret = Wifi_dequeue(&message_tmp, portMAX_DELAY);
		if(ret == pdPASS){
			switch(message_tmp.addr_src){
				case ADDR_MCU:
					LOG("ADDR_MCU send msg to wifi\r\n");
					break;
			}
		}else if(ret == errQUEUE_EMPTY){
			LOG("WIFI_dequeue errQUEUE_EMPTY\r\n");
		}
	}
}