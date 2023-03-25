#include "ourLibrary.h"
//File handles all CAN BUS related functionality

void CAN_RX_ISR (void) {
	uint8_t RX_Message_ISR[8];
	uint32_t ID;
	CAN_RX(ID, RX_Message_ISR);
	xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}

void CANSend(void *pvParameters) {
	uint8_t msgOut[8];
	while (1) {
		//Code is blocking so must disable during timing analysis
		#ifndef TEST_MODE
			xQueueReceive(msgOutQ, msgOut, portMAX_DELAY);
		#endif
		xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);
		//Code is blocking so must disable during timing analysis
		#ifndef TEST_MODE
			CAN_TX(MASTER_ID, msgOut);
		#endif
		xSemaphoreGive(CAN_TX_Semaphore);
		//Time analysis
        #ifdef TEST_MODE
            return;
        #endif
	}
}

void CAN_TX_ISR(void) {
	xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
}

void decodeTask(void *pvParamters) {
	uint8_t RX_Message[8];
	while (1) {
		//Call is blocking, so must disable during time analysis
		#ifndef TEST_MODE
			xQueueReceive(msgInQ, RX_Message, portMAX_DELAY);
		#endif
		if (!ISMASTER) {
			continue;
		}
		// Process the received message
		if (RX_Message[0] == 'P') {
			allocAccumulator(RX_Message[2], RX_Message[1]);
		} else {
			deallocAccumulator(RX_Message[2], RX_Message[1]);
		}
		//Time analysis
        #ifdef TEST_MODE
            return;
        #endif
  	}
}
