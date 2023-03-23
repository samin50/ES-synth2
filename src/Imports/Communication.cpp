#include "ourLibrary.h"
//File handles all CAN BUS related functionality

void CAN_RX_ISR (void) {
	uint8_t RX_Message_ISR[8];
	uint32_t ID;
	CAN_RX(ID, RX_Message_ISR);
	xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}

void CANSend(void * pvParameters) {
	uint8_t msgOut[8];
	while (1) {
		xQueueReceive(msgOutQ, msgOut, portMAX_DELAY);
		xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);
		CAN_TX(MASTER_ID, msgOut);
	}
}

void CAN_TX_ISR(void) {
	xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
}

void decodeTask(void *pvParamters) {
	const TickType_t xFrequency = 50/portTICK_PERIOD_MS;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	uint8_t RX_Message[8];
	while (1) {
		xQueueReceive(msgInQ, RX_Message, portMAX_DELAY);
		if (!ISMASTER) {
			continue;
		}
		//If the same octave, reject packet to prevent freezing
		if (RX_Message[1] == OCTAVE) {
			continue;
		}
		// Process the received message
		if (RX_Message[0] == 'P') {
			allocAccumulator(RX_Message[2], RX_Message[1]);
		} else {
			deallocAccumulator(RX_Message[2], RX_Message[1]);
		}
  	}
}
