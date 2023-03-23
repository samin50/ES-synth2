#include "ourLibrary.H"

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

void stateChange(uint8_t prevKeys[], uint8_t currKeys[]){
	//Disable registering keypresses during playback
	if (ISPLAYBACK) {
		return;
	}
	uint8_t prevKeyRow;
	uint8_t currKeyRow;
	uint8_t columnIndex = 0;
	uint8_t keyNum = 0;
	uint8_t TX_Message[8] = {0};
	for (int i=0; i<3;i++){
		prevKeyRow = prevKeys[i];
		currKeyRow = currKeys[i];
		columnIndex = 0;
		keyNum = 0;
		for (int columnIndex = 0; columnIndex < 4; columnIndex++) {
			keyNum = (i*4)+columnIndex;
			if (((prevKeyRow&1)^(currKeyRow&1))==1) {
				//State change in keys
				if ((prevKeyRow&1) == 0) { 
					//key Released 	
					TX_Message[0] = 'R';
					//If master, dealloc accumulator directly 
					if (ISMASTER) {
						deallocAccumulator(keyNum, OCTAVE);
					}
				} else {			   
					//key Pressed	
					TX_Message[0] = 'P';
					//If master, alloc accumulator directly
					if (ISMASTER) {
						allocAccumulator(keyNum, OCTAVE);
					}
				}
				TX_Message[1] = OCTAVE;
				TX_Message[2] = keyNum;
				//If not master, send to master
				if (!ISMASTER) {
					xQueueSend(msgOutQ, TX_Message, portMAX_DELAY);
				}
			}
			currKeyRow = currKeyRow >> 1;
			prevKeyRow = prevKeyRow >> 1;
		}
	}
}
